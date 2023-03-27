#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"
#include <concurrent_priority_queue.h>
//#include <DirectXMath.h>

enum EVENT_TYPE { EV_RANDOM_MOVE };

struct TIMER_EVENT {
	int room_id;
	int obj_id;
	high_resolution_clock::time_point wakeup_time;
	int event_id;
	int target_id;
	constexpr bool operator < (const TIMER_EVENT& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

array<array<SESSION, MAX_USER_PER_ROOM>, MAX_ROOM> clients;
//array<array<Monster, MAX_MONSTER_PER_ROOM>, MAX_ROOM> monsters;
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

void Initialize_Monster(int roomNum, int stageNum)//0326
{
	for (auto& info : StagesInfo[stageNum - 1]) {
		Monster* M = MonsterPool.GetMemory();
		M->Initialize(roomNum, info.id, info.type, info.Pos);
		PoolMonsters[roomNum].emplace_back(M);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			if (clients[roomNum][i]._state == ST_INGAME) {
				clients[roomNum][i].cur_stage = stageNum;
				clients[roomNum][i].send_summon_monster_packet(M);
				TIMER_EVENT ev{ roomNum, M->m_id, high_resolution_clock::now(), EV_RANDOM_MOVE, -1 };
				timer_queue.push(ev);
				//cout << roomNum << "번 방 " << stageNum << " 스테이지 몬스터 소환\n";
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
	//else
	//{
	//	for (MapObject*& object : Objects[(int)newPos.z / 600]) {
	//		if (object->m_xmOOBB.Contains(BoundingBox(newPos, { FLT_EPSILON,FLT_EPSILON ,FLT_EPSILON }))) {
	//			cout << object->m_pstrName << " COLLIDED\n";
	//			overwrite = true;
	//			return;
	//		}
	//	}
	//}


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



void SESSION::Update()
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
	clock_t start_time = clock();
	while (!openList.empty())
	{
		//if (duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count() >= 1000)
		//{
		//	cout << start_Pos.x << ", " << start_Pos.y << ", " << start_Pos.z << "  to  " << dest_Pos.x << ", " << dest_Pos.y << ", " << dest_Pos.z << "추적 중지\n";
		//	cur_animation_track = 0;
		//	target_id = -1;
		//	SetState(NPC_State::Idle);
		//	return Pos;
		//}
		iter = getNode(&openList);
		S_Node = *iter;
		if (BoundingBox(S_Node->Pos, { 5,20,5 }).Intersects(clients[room_num][target_id].m_xmOOBB))
		{
			while (S_Node->parent != nullptr)
			{
				if (Vector3::Compare2D(S_Node->parent->Pos, start_Pos))
				{
					cout << "찾는 데 걸린 시간 - " << clock() - start_time << endl;
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

	// 몬스터와 플레이어 사이의 벡터
	XMVECTOR distanceVector = DirectX::XMLoadFloat3(&Pos) - DirectX::XMLoadFloat3(&clients[room_num][target_id].GetPosition());

	// 몬스터와 플레이어 사이의 거리 계산
	g_distance = XMVectorGetX(XMVector3Length(distanceVector));

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
		else
		{
			if (0 == type)
			{
				if (150 > g_distance)
				{
					SetState(NPC_State::Attack);
					cur_animation_track = 2;
				}
				roadToMove = stack<XMFLOAT3>(); // 스택 초기화
			}
		}
		if (!roadToMove.empty())
		{
			Pos = roadToMove.top();
			roadToMove.pop();
		}
		else {

			int collide_range = (int)(Pos.z / 600);
			XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(Vector3::RemoveY(Vector3::Normalize(Vector3::Subtract(clients[room_num][target_id].GetPosition(), Pos))), speed, false));
			if (0 == type)
			{
				//cout << "dis : " << g_distance << endl;

				if (150 <= g_distance)//150이면 stop
				{
					Pos = newPos;
					BB.Center = Pos;
				}
			}
			else
			{
				if (Objects[collide_range].end() == find_if(Objects[collide_range].begin(), Objects[collide_range].end(), [newPos](MapObject* Obj) {return BoundingBox(newPos, { 5,3,5 }).Intersects(Obj->m_xmOOBB); })) {
					Pos = newPos;
					BB.Center = Pos;
				}
				else {
					Pos = Find_Direction(Pos, clients[room_num][target_id].GetPosition());
					BB.Center = Pos;
				}
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
		if (GetAttackTimer() <= 0) {//0326
			if (0 == type)
			{
				if (150 <= g_distance)//마법 공격x
				{
					SetState(NPC_State::Chase);
					cur_animation_track = 1;
					SetAttackTimer(attack_cycle);
				}
				else//마법 공격0
				{
					MagicPos = Vector3::Add(GetPosition(), XMFLOAT3(0, 10, 0));
					MagicLook = GetLookVector();

					MagicPos = Vector3::Add(MagicPos, Vector3::ScalarProduct(MagicLook, 10, false));
		

					if (BoundingBox(MagicPos, { 200,200,200 }).Intersects(clients[room_num][target_id].m_xmOOBB))
					{
						lock_guard <mutex> ll{ clients[room_num][target_id]._s_lock };
						clients[room_num][target_id].HP -= GetPower();
						

						float plMass = 1.0f;
						float magicMass = 20.0f;
						XMFLOAT3 plPos = clients[room_num][target_id].GetPosition();
						XMFLOAT3 magicTempPos = MagicPos;
						//XMFLOAT3 magicTempLook = MagicLook;

						// 플레이어와 마법의 벡터 방향을 구합니다.
						XMVECTOR collisionDir = XMVectorSubtract(DirectX::XMLoadFloat3(&plPos), DirectX::XMLoadFloat3(&magicTempPos));
						collisionDir = XMVector3Normalize(collisionDir);
						//DirectX::XMLoadFloat3(&magicTempLook)= XMVector3Normalize(DirectX::XMLoadFloat3(&magicTempLook));

						// 플레이어에 가해질 힘을 계산합니다.
						float forceMagnitude = magicMass * 10.0f;
						//XMVECTOR forceVector = XMVectorScale(DirectX::XMLoadFloat3(&magicTempLook), forceMagnitude);
						XMVECTOR forceVector = XMVectorScale(collisionDir, forceMagnitude);

						// 플레이어의 속도를 계산합니다.
						float velocityMagnitude = forceMagnitude / plMass;
						//XMVECTOR velocityVector = XMVectorScale(DirectX::XMLoadFloat3(&magicTempLook), velocityMagnitude);
						XMVECTOR velocityVector = XMVectorScale(collisionDir, velocityMagnitude);

						// 플레이어의 위치와 속도를 업데이트합니다.
						DirectX::XMStoreFloat3(&clients[room_num][target_id].m_xmf3Position,XMVectorAdd(DirectX::XMLoadFloat3(&plPos), velocityVector));
						//send_move_packet()
						XMFLOAT3 velTmp;
						DirectX::XMStoreFloat3(&velTmp, velocityVector);

						cout << "velocityVector x: " << velTmp.x << endl;
						cout << "velocityVector y: " << velTmp.y << endl;
						cout << "velocityVector z: " << velTmp.z << endl;
						cout << "plPos x:" << plPos.x << endl;
						cout << "plPos x:" << plPos.y << endl;
						cout << "plPos x:" << plPos.z << endl;// << endl << endl;

						/*cout << "MagicPos x: " << MagicPos.x << "	pl x:" << clients[room_num][target_id].GetPosition().x << endl;
						cout << "MagicPos y: " << MagicPos.y << "	pl y:" << clients[room_num][target_id].GetPosition().y << endl;
						cout << "MagicPos z: " << MagicPos.z << "	pl z:" << clients[room_num][target_id].GetPosition().z << endl;*/
						cout << "plHP magic : " << clients[room_num][target_id].HP << endl << endl << endl;

						MagicPos = XMFLOAT3(5000, 5000, 5000);
					}
				}
			}
			else
			{
				if (!BB.Intersects(clients[room_num][target_id].m_xmOOBB)) {//부딪히지x
					SetState(NPC_State::Chase);
					cur_animation_track = 1;
					SetAttackTimer(attack_cycle);
				}
				else {//부딪혔을 때

					lock_guard <mutex> ll{ clients[room_num][target_id]._s_lock };
					clients[room_num][target_id].HP -= GetPower();

					cout << "plHP : " << clients[room_num][target_id].HP << endl;
				}
			}
			SetAttackTimer(attack_cycle);
		}
		break;
	case NPC_State::Dead:
		cur_animation_track = 3;
		dead_timer -= fTimeElapsed;
		if (dead_timer <= 0) {
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
			StagesInfo[2].push_back(MonsterInfo(XMFLOAT3(100.f + i * 10, -17.5, 1900), 3, ID_constructor++));
		}
	}
	{	// 4stage
	}
	{	// 5stage

	}
	{	// 6stage

	}
}

void monster_update(int roomNum, int monster_id)
{
	auto iter = find_if(PoolMonsters[roomNum].begin(), PoolMonsters[roomNum].end(), [monster_id](Monster* M) {return M->m_id == monster_id; });
	if ((*iter)->HP <= 0 && (*iter)->GetState() != NPC_State::Dead) {
		(*iter)->SetState(NPC_State::Dead);
	}

	switch ((*iter)->GetState())
	{
	case NPC_State::Idle:
		(*iter)->target_id = (*iter)->get_targetID();
		if ((*iter)->target_id != -1) {
			(*iter)->SetState(NPC_State::Chase);
			(*iter)->cur_animation_track = 1;
		}
		break;
	case NPC_State::Chase:
		if ((*iter)->BB.Intersects(clients[roomNum][(*iter)->target_id].m_xmOOBB)) {
			(*iter)->SetState(NPC_State::Attack);
			if ((*iter)->getType() != 2) {
				(*iter)->cur_animation_track = 2;
			}
			(*iter)->roadToMove = stack<XMFLOAT3>(); // 스택 초기화
		}
		if (!(*iter)->roadToMove.empty())
		{
			(*iter)->Pos = (*iter)->roadToMove.top();
			(*iter)->roadToMove.pop();
		}
		else {
			int collide_range = (int)((*iter)->Pos.z / 600);
			XMFLOAT3 newPos = Vector3::Add((*iter)->Pos, Vector3::ScalarProduct(Vector3::RemoveY(Vector3::Normalize(Vector3::Subtract(clients[roomNum][(*iter)->target_id].GetPosition(), (*iter)->Pos))), (*iter)->GetSpeed(), false));
			if (Objects[collide_range].end() == find_if(Objects[collide_range].begin(), Objects[collide_range].end(), [newPos](MapObject* Obj) {return BoundingBox(newPos, { 5,3,5 }).Intersects(Obj->m_xmOOBB); })) {
				(*iter)->Pos = newPos;
				(*iter)->BB.Center = (*iter)->Pos;
			}
			else {
				(*iter)->Pos = (*iter)->Find_Direction((*iter)->Pos, clients[roomNum][(*iter)->target_id].GetPosition());
				(*iter)->BB.Center = (*iter)->Pos;
			}
		}
		break;
	case NPC_State::Attack:
		(*iter)->attack_timer -= 0.01;
		if (clients[roomNum][(*iter)->target_id].HP <= 0) {
			(*iter)->SetState(NPC_State::Idle);
			(*iter)->cur_animation_track = 0;
			(*iter)->target_id = -1;
			(*iter)->SetAttackTimer((*iter)->attack_cycle);
			return;
		}
		if ((*iter)->GetAttackTimer() <= 0) {
			if (!(*iter)->BB.Intersects(clients[roomNum][(*iter)->target_id].m_xmOOBB)) {
				(*iter)->SetState(NPC_State::Chase);
				(*iter)->cur_animation_track = 1;
				(*iter)->SetAttackTimer((*iter)->attack_cycle);
			}
			else {
				lock_guard <mutex> ll{ clients[roomNum][(*iter)->target_id]._s_lock };
				clients[roomNum][(*iter)->target_id].HP -= (*iter)->GetPower();
			}
			(*iter)->SetAttackTimer((*iter)->attack_cycle);
		}
		break;
	case NPC_State::Dead:
		(*iter)->cur_animation_track = 3;
		(*iter)->dead_timer -= 0.01;
		if ((*iter)->dead_timer <= 0) {
			(*iter)->SetAlive(false);
		}
		break;
	default:
		break;
	}
}