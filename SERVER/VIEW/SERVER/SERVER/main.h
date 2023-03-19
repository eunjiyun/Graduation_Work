#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"


array<array<SESSION, MAX_USER_PER_ROOM>, MAX_ROOM> clients;
array<array<Monster, MAX_MONSTER_PER_ROOM>, MAX_ROOM> monsters;

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

void SESSION::send_summon_monster_packet(int npc_id)
{
	SC_SUMMON_MONSTER_PACKET summon_packet;
	summon_packet.id = npc_id;
	summon_packet.size = sizeof(summon_packet);
	summon_packet.type = SC_SUMMON_MONSTER;
	summon_packet.Pos = monsters [_id / 4][npc_id].GetPosition();
	summon_packet.monster_type = monsters[_id / 4][npc_id].getType();
	do_send(&summon_packet);
}

void SESSION::send_NPCUpdate_packet(int npc_id)
{
	SC_MOVE_MONSTER_PACKET p;
	p.id = npc_id;
	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
	p.type = SC_MOVE_MONSTER;
	p.Pos = monsters[_id / 4][npc_id].GetPosition();
	p.HP = monsters[_id / 4][npc_id].HP;
	p.animation_track = monsters[_id / 4][npc_id].cur_animation_track;
	p.Chasing_PlayerID = monsters[_id / 4][npc_id].target_id;
	do_send(&p);
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


}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER / MAX_USER_PER_ROOM; ++i) {
		for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
			lock_guard <mutex> ll{ clients[i][j]._s_lock };
			if (clients[i][j]._state == ST_FREE)
				return i * 4 + j;
		}
	}
	return -1;
}

int Initialize_Monster(int roomNum, int stageNum)
{
	int monster_count = 0;
	switch (stageNum)
	{
	case 1:
		monster_count = 3;
		for (int i = 0; i < monster_count; ++i) {
			monsters[roomNum][i].Initialize(roomNum, 0, { -100.f + 20.f * i, -17.5f, 700.f + i * 60.f });
		}
		break;
	case 2:
		monster_count = 3;
		for (int i = 0; i < monster_count; ++i) {
			monsters[roomNum][i].Initialize(roomNum, 1, { -100.f + 20.f * i, -17.5f, 700.f + i * 60.f });
		}
		break;
	case 3:
		monster_count = 4;
		for (int i = 0; i < monster_count; ++i) {
			monsters[roomNum][i].Initialize(roomNum, 2, { -170.f + 50.f * i, -17.5f, 1800.f });
		}
		break;
	}
	return monster_count;
}

void SESSION::Update()
{
	Move(direction, 21.0f, true);

	if (onAttack)
	{
		switch (character_num)
		{
		case 0:
			for (auto& monster : monsters[_id / 4]) {
				if (monster.HP > 0 && BoundingBox(GetPosition(), { 10,3,10 }).Intersects(monster.BB))
				{
					monster.HP -= 50;
				}
			}
			break;
		case 1:
			//if (onShooting = false) {
			//	BulletPos = GetPosition();
			//	onShooting = true;
			//}
			break;
		case 2:
			for (auto& monster : monsters[_id / 4]) {
				if (monster.HP > 0 && BoundingBox(GetPosition(), { 10,3,10 }).Intersects(monster.BB))
				{
					monster.HP -= 50;
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
	//if (onShooting) {
	//	BulletPos = Vector3::Add(BulletPos, Vector3::ScalarProduct(GetLookVector(), 10, false));
	//	for (auto& monster : monsters[_id / 4]) {
	//		if (monster.HP > 0 && BoundingBox(BulletPos, { 1,1,3 }).Intersects(monster.BB))
	//		{
	//			monster.HP -= 150;
	//			BulletPos = { 5000,5000,5000 };
	//			onShooting = false;
	//		}
	//	}
	//}
	short stage = GetPosition().z / 600;

	if (stage > cur_stage) {
		int monster_count = Initialize_Monster(_id / 4, stage);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			for (int j = 0; j < monster_count; ++j) {
				if (clients[_id / 4][i]._state == ST_INGAME) {
					clients[_id / 4][i].send_summon_monster_packet(j);
					clients[_id / 4][i].cur_stage = stage;
					cout << _id / 4 << "번 방 " << stage << " 스테이지 몬스터 소환\n";
				}
			}
		}
	}

}
bool check_path(XMFLOAT3 _pos, vector<XMFLOAT3> CloseList)
{
	int collide_range = _pos.z / 600;
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
	clock_t start_time = clock();
	while (!openList.empty())
	{
		if (clock() - start_time >= 1000)
		{
			cout << start_Pos.x << ", " << start_Pos.y << ", " << start_Pos.z << "  to  " << dest_Pos.x << ", " << dest_Pos.y << ", " << dest_Pos.z << "추적 중지\n";
			cur_animation_track = 0;
			target_id = -1;
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
					if (clock() - start_time > 100)
						cout << "수행시간: " <<  clock() - start_time << endl;
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

	int min = *min_element(distances.begin(), distances.end());

	if (min < view_range)
	{
		return min_element(distances.begin(), distances.end()) - distances.begin();
	}
	else return -1;
}

void Monster::Update(float fTimeElapsed)
{
	is_alive = HP + abs(HP);
	switch (curState)
	{
	case NPC_State::Idle:
		target_id = get_targetID();
		if (target_id != -1) {
			curState = NPC_State::Chase;
			cur_animation_track = 1;
		}
		break;
	case NPC_State::Chase:
		if (BB.Intersects(clients[room_num][target_id].m_xmOOBB)) {
			curState = NPC_State::Attack;
			if (type != 3) {
				cur_animation_track = 2;
			}
		}
		if (!roadToMove.empty())
		{
			Pos = roadToMove.top();
			roadToMove.pop();
		}
		else {
			int collide_range = Pos.z / 600;
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
			curState = NPC_State::Idle;
			cur_animation_track = 0;
			target_id = -1;
			attack_timer = 1.f;
			return;
		}
		if (attack_timer <= 0) {
			{
				lock_guard <mutex> ll{ clients[room_num][target_id]._s_lock };
				clients[room_num][target_id].HP -= GetPower();
			}
			attack_timer = attack_cycle;
		}
		if (!BB.Intersects(clients[room_num][target_id].m_xmOOBB)) {
			curState = NPC_State::Chase;
			cur_animation_track = 1;
			attack_timer = 1.f;
		}
		break;
	case NPC_State::Dead:
		break;
	default:
		break;
	}
	//if (target_id < 0) {
	//	cur_animation_track = 0;
	//	target_id = get_targetID();
	//	return;
	//}
	//if (BB.Intersects(clients[room_num][target_id].m_xmOOBB))
	//{
	//	if (type != 3) {
	//		cur_animation_track = 2;
	//	}
	//	return;
	//}

	//cur_animation_track = 1;
	//// 플레이어 추적
	//if (!roadToMove.empty())
	//{
	//	Pos = roadToMove.top();
	//	roadToMove.pop();
	//}
	//else {
	//	int collide_range = Pos.z / 600;
	//	XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(Vector3::RemoveY(Vector3::Normalize(Vector3::Subtract(clients[room_num][target_id].GetPosition(), Pos))), speed, false));
	//	if (Objects[collide_range].end() == find_if(Objects[collide_range].begin(), Objects[collide_range].end(), [newPos](MapObject* Obj) {return BoundingBox(newPos, { 5,3,5 }).Intersects(Obj->m_xmOOBB); })) {
	//		Pos = newPos;
	//		BB.Center = Pos;
	//	}
	//	else {
	//		Pos = Find_Direction(Pos, clients[room_num][target_id].GetPosition());
	//		BB.Center = Pos;
	//	}
	//}
}

