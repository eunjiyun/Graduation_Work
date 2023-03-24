#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"


array<array<SESSION, MAX_USER_PER_ROOM>, MAX_ROOM> clients;
array<array<Monster, MAX_MONSTER_PER_ROOM>, MAX_ROOM> monsters;
array<vector<Monster*>, MAX_ROOM> PoolMonsters;
CObjectPool<Monster> MonsterPool(10'000);
array<vector<MonsterInfo>, 6> StagesInfo;



MapObject** m_ppObjects = 0;
vector<MapObject*> Objects[6] = {};
int m_nObjects = 0;

SESSION* getClient(int c_id)
{
	return &clients[c_id / 4][c_id % 4];
}


void disconnect(int c_id)
{
	for (auto& pl : clients[c_id / 4]) {
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
	SESSION* CL = getClient(c_id);
	closesocket(CL->_socket);

	lock_guard<mutex> ll(CL->_s_lock);
	CL->_state = ST_FREE;
}

void SESSION::send_summon_monster_packet(Monster* M)
{
	SC_SUMMON_MONSTER_PACKET summon_packet;
	summon_packet.id = M->m_id;
	summon_packet.size = sizeof(summon_packet);
	summon_packet.type = SC_SUMMON_MONSTER;
	summon_packet.Pos = M->GetPosition();
	summon_packet.monster_type = M->getType();
	do_send(&summon_packet);
}

void SESSION::send_NPCUpdate_packet(Monster* M)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = M->m_id;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.type = SC_MOVE_MONSTER;
	p.Pos = M->GetPosition();
	p.HP = M->HP;
	p.is_alive = M->is_alive();
	p.animation_track = M->cur_animation_track;
	p.Chasing_PlayerID = M->target_id;
	do_send(&p);
}

void Initialize_Monster(int roomNum, int stageNum)
{
	for (auto& info : StagesInfo[stageNum - 1]) {
		Monster* M = MonsterPool.GetMemory();
		M->Initialize(roomNum, info.id, info.type, info.Pos);
		PoolMonsters[roomNum].emplace_back(M);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			if (clients[roomNum][i]._state == ST_INGAME) {
				clients[roomNum][i].cur_stage = stageNum;
				clients[roomNum][i].send_summon_monster_packet(M);
				cout << roomNum << "번 방 " << stageNum << " 스테이지 몬스터 소환\n";
			}
		}
	}
}

void SESSION::CheckPosition(XMFLOAT3 newPos)
{
	// 이동속도가 말도 안되게 빠른 경우 체크
	XMFLOAT3 Distance = Vector3::Subtract(newPos, GetPosition());
	if (sqrtf(Distance.x * Distance.x + Distance.z * Distance.z) > 100.f) {
		cout << "client[" << _id << "] 에러 포인트 감지\n";
		overwrite = true;
		return;
	}
	else
	{
		for (MapObject*& object : Objects[(int)newPos.z / 600]) {
			if (object->m_xmOOBB.Contains(BoundingBox(newPos, { FLT_EPSILON,FLT_EPSILON ,FLT_EPSILON }))) {
				overwrite = true;
				return;
			}
		}
	}


	overwrite = false;
	SetPosition(newPos);
	UpdateBoundingBox();

	short stage = (short)(GetPosition().z / 600);

	if (stage > cur_stage) {
		Initialize_Monster(_id / 4, stage);
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_ROOM; ++i) {
		for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
			lock_guard <mutex> ll{ clients[i][j]._s_lock };
			if (clients[i][j]._state == ST_FREE)
				return i * 4 + j;
		}
	}
	return -1;
}



void SESSION::Update(float fTimeElapsed)
{
	Move(direction, 21.0f, true);

	if (onAttack)
	{
		switch (character_num)
		{
		case 0:
			for (auto& monster : PoolMonsters[_id / 4]) {
				if (monster->HP > 0 && BoundingBox(GetPosition(), { 15,1,15 }).Intersects(monster->BB))
				{
					monster->HP -= 100;
				}
			}
			break;
		case 1:
			BulletPos = Vector3::Add(GetPosition(), XMFLOAT3(0, 10, 0));
			BulletLook = GetLookVector();
			break;
		case 2:
			for (auto& monster : PoolMonsters[_id / 4]) {
				if (monster->HP > 0 && BoundingBox(GetPosition(), { 5,1,5 }).Intersects(monster->BB))
				{
					monster->HP -= 50;
				}
			}
			break;
		}
	}
	if (onCollect)
	{

	}
	if (HP <= 0)
	{
		direction = DIR_DIE;
	}

	BulletPos = Vector3::Add(BulletPos, Vector3::ScalarProduct(BulletLook, 10, false));
	for (auto& monster : PoolMonsters[_id / 4]) {
		if (monster->HP > 0 && BoundingBox(BulletPos, { 10,10,10 }).Intersects(monster->BB))
		{

			monster->HP -= 200;
			BulletPos = XMFLOAT3(5000, 5000, 5000);
			break;
		}
	}
}
bool check_path(XMFLOAT3 _pos, vector<XMFLOAT3> CloseList)
{
	int collide_range = (int)(_pos.z / 600);
	BoundingBox CheckBox = BoundingBox(_pos, { 5,3,5 });

	if ((CloseList.end() != find_if(CloseList.begin(), CloseList.end(), [&_pos](XMFLOAT3 pos_) {return Vector3::Compare(_pos, pos_); })) ||
		Objects[collide_range].end() != find_if(Objects[collide_range].begin(), Objects[collide_range].end(), [&_pos](MapObject* Obj) {return BoundingBox(_pos, { 5,3,5 }).Intersects(Obj->m_xmOOBB); }))
		return false;

	return true;
}
list<A_star_Node*>::iterator getNode(list<A_star_Node*>* m_List)
{
	return min_element(m_List->begin(), m_List->end(), [](A_star_Node* N1, A_star_Node* N2) {return N1->F < N2->F; });
}
bool check_openList(XMFLOAT3 _Pos, float _G, A_star_Node* s_node, list<A_star_Node*>* m_List)
{
	auto iter = find_if((*m_List).begin(), (*m_List).end(), [&_Pos](A_star_Node* N) {return Vector3::Compare2D(_Pos, N->Pos); });
	if (iter != (*m_List).end()) {
		if ((*iter)->G > _G) {
			(*iter)->G = _G;
			(*iter)->F = (*iter)->G + (*iter)->H;
			(*iter)->parent = s_node;
		}
		return false;
	}
	return true;
}

float nx[8]{ -1,1,0,0, -1, -1, 1, 1 };
float nz[8]{ 0,0,1,-1, -1, 1, -1, 1 };
XMFLOAT3 Monster::Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos)
{
	vector<XMFLOAT3> CloseList{};
	list<A_star_Node*> openList;
	A_star_Node* S_Node;

	openList.push_back(new A_star_Node(start_Pos, dest_Pos));
	list<A_star_Node*>::iterator iter;
	auto start_time = high_resolution_clock::now();
	while (!openList.empty())
	{
		if (duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() >= 1000)
		{
			cout << start_Pos.x << ", " << start_Pos.y << ", " << start_Pos.z << "  to  " << dest_Pos.x << ", " << dest_Pos.y << ", " << dest_Pos.z << "추적 중지\n";
			cur_animation_track = 0;
			target_id = -1;
			SetState(NPC_State::Idle);
			return Pos;
		}
		iter = getNode(&openList);
		S_Node = *iter;
		if (BoundingBox(S_Node->Pos, { 5,20,5 }).Intersects(clients[room_num][target_id].m_xmOOBB))
		{
			while (S_Node->parent != nullptr)
			{
				if (Vector3::Compare2D(S_Node->parent->Pos, start_Pos))
				{
					return S_Node->Pos;
				}
				roadToMove.push(S_Node->Pos);
				S_Node = S_Node->parent;
			}
		}
		for (int i = 0; i < 8; i++) {
			XMFLOAT3 _Pos = Vector3::Add(S_Node->Pos, Vector3::ScalarProduct(XMFLOAT3{ nx[i],0,nz[i] }, speed, false));

			if (check_path(_Pos, CloseList) && check_openList(_Pos, S_Node->G + speed * sqrt(abs(nx[i]) + abs(nz[i])), S_Node, &openList)) {
				openList.push_back(new A_star_Node(_Pos, dest_Pos, S_Node->G + speed * sqrt(abs(nx[i]) + abs(nz[i])), S_Node));
			}
		}
		CloseList.push_back(S_Node->Pos);
		openList.erase(iter);
	}	
	cur_animation_track = 0;
	return Pos;
}

int Monster::get_targetID()
{
	for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
		if (clients[room_num][i]._state != ST_INGAME) {
			distances[i] = view_range;
			continue;
		}
		float distance_z = clients[room_num][i].GetPosition().z - Pos.z;
		float distance_x = clients[room_num][i].GetPosition().x - Pos.x;
		distances[i] = sqrtf(distance_z * distance_z + distance_x * distance_x);
	}

	float min = *min_element(distances.begin(), distances.end());

	if (min < view_range)
	{
		return min_element(distances.begin(), distances.end()) - distances.begin();
	}
	else return -1;
}

void Monster::Update(float fTimeElapsed)
{
	if (HP <= 0 && GetState() != NPC_State::Dead) {
		SetState(NPC_State::Dead);
	}

	switch (GetState())
	{
	case NPC_State::Idle:
		target_id = get_targetID();
		if (target_id != -1) {
			SetState(NPC_State::Chase);
			cur_animation_track = 1;
		}
		break;
	case NPC_State::Chase:
		if (BB.Intersects(clients[room_num][target_id].m_xmOOBB)) {
			SetState(NPC_State::Attack);
			if (type != 2) {
				cur_animation_track = 2;
			}
			roadToMove = stack<XMFLOAT3>(); // 스택 초기화
		}
		if (!roadToMove.empty())
		{
			Pos = roadToMove.top();
			roadToMove.pop();
		}
		else {
			int collide_range = (int)(Pos.z / 600);
			XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(Vector3::RemoveY(Vector3::Normalize(Vector3::Subtract(clients[room_num][target_id].GetPosition(), Pos))), speed, false));
			if (Objects[collide_range].end() == find_if(Objects[collide_range].begin(), Objects[collide_range].end(), [newPos](MapObject* Obj) {return BoundingBox(newPos, { 5,3,5 }).Intersects(Obj->m_xmOOBB); })) {
				Pos = newPos;
				BB.Center = Pos;
			}
			else {
				Pos = Find_Direction(Pos, clients[room_num][target_id].GetPosition());
				BB.Center = Pos;
			}
		}
		break;
	case NPC_State::Attack:
		attack_timer -= fTimeElapsed;
		if (clients[room_num][target_id].HP <= 0) {
			SetState(NPC_State::Idle);
			cur_animation_track = 0;
			target_id = -1;
			SetAttackTimer(attack_cycle);
			return;
		}
		if (GetAttackTimer() <= 0) {
			if (!BB.Intersects(clients[room_num][target_id].m_xmOOBB)) {
				SetState(NPC_State::Chase);
				cur_animation_track = 1;
				SetAttackTimer(attack_cycle);
			}
			else {
				lock_guard <mutex> ll{ clients[room_num][target_id]._s_lock };
				clients[room_num][target_id].HP -= GetPower();
			}
			SetAttackTimer(attack_cycle);
		}
		break;
	case NPC_State::Dead:
		 cur_animation_track = 3;
		 dead_timer -= fTimeElapsed;
		 if (dead_timer <= 0 ) {
			 SetAlive(false);
		 }
		break;
	default:
		break;
	}
}


void InitializeStages()
{
	int ID_constructor = 0;
	{	// 1stage
		for (int i = 0; i < 4; ++i) {
			StagesInfo[0].push_back(MonsterInfo(XMFLOAT3(-100.f + i * 50, -17.5, 650), 0, ID_constructor++));
	
		}
	}
	{	// 2stage
		for (int i = 0; i < 3; ++i) {
			StagesInfo[1].push_back(MonsterInfo(XMFLOAT3(-100.f + i * 50, -17.5, 1000), 1, ID_constructor++));
		}
	}
	{	// 3stage
		for (int i = 0; i < 3; ++i) {
			StagesInfo[2].push_back(MonsterInfo(XMFLOAT3(100.f + i * 10, -17.5, 1900), 2, ID_constructor++));
		}
	}
	{	// 4stage

	}
	{	// 5stage

	}
	{	// 6stage

	}
}