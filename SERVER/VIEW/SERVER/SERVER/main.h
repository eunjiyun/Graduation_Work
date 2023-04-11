#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"


enum EVENT_TYPE { EV_RANDOM_MOVE };

struct TIMER_EVENT {
	int room_id;
	int obj_id;
	high_resolution_clock::time_point wakeup_time;
	int event_id;
	constexpr bool operator < (const TIMER_EVENT& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};
concurrent_priority_queue<TIMER_EVENT> timer_queue;

array<array<SESSION, MAX_USER_PER_ROOM>, MAX_ROOM> clients;
array<vector<Monster*>, MAX_ROOM> PoolMonsters;


CObjectPool<Monster> MonsterPool(50'000);
array<vector<MonsterInfo>, 10> StagesInfo;

int check_pathTime = 0;
int check_openListTime = 0;

array<vector< MapObject*>, 42> Objects;


SESSION& getClient(int c_id)
{
	return clients[c_id / 4][c_id % 4];
}

array<SESSION, MAX_USER_PER_ROOM>& getRoom(int c_id)
{
	return clients[c_id / 4];
}

vector<Monster*>& getMonsters(int c_id)
{
	return PoolMonsters[c_id / 4];
}

vector<MapObject*>& getPartialObjects(XMFLOAT3 Pos)
{
	return Objects[(int)Pos.z / STAGE_SIZE];
}


void disconnect(int c_id)
{
	bool in_game = false;
	for (auto& pl : getRoom(c_id)) {

		if (ST_INGAME != pl._state.load() && ST_DEAD != pl._state.load()) continue;
		if (pl._id == c_id) continue;

		pl.send_remove_player_packet(c_id);
		in_game = true;
	}
	SESSION& CL = getClient(c_id);
	closesocket(CL._socket);

	if (in_game) // 아직 방 안에 사람이 있다면 
		CL._state.store(ST_CRASHED);

	else { // 방이 비었다면 몬스터와 플레이어 컨테이너 모두 정리
		for (auto& pl : getRoom(c_id)) {
			pl._state.store(ST_FREE);
		}
		for (auto iter = PoolMonsters[c_id / 4].begin(); iter != PoolMonsters[c_id / 4].end();) {
			lock_guard<mutex> mm{ (*iter)->m_lock };
			MonsterPool.ReturnMemory(*iter);
			PoolMonsters[c_id / 4].erase(iter);
		}
	}
}

void SESSION::send_summon_monster_packet(Monster* M)
{
	SC_SUMMON_MONSTER_PACKET summon_packet;
	summon_packet.id = M->m_id;
	summon_packet.size = sizeof(summon_packet);
	summon_packet.type = SC_SUMMON_MONSTER;
	summon_packet.Pos = M->GetPosition();
	summon_packet.room_num = M->room_num;
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
	p.BulletPos = M->MagicPos;
	p.room_num = M->room_num;
	do_send(&p);
}

void Initialize_Monster(int roomNum, int stageNum)//0326
{
	for (auto& info : StagesInfo[stageNum - 1]) {
		Monster* M = MonsterPool.GetMemory();
		M->Initialize(roomNum, info.id, info.type, info.Pos);
		PoolMonsters[roomNum].emplace_back(M);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			if (clients[roomNum][i]._state.load() == ST_INGAME || clients[roomNum][i]._state.load() == ST_DEAD) {
				clients[roomNum][i].cur_stage = stageNum;
				clients[roomNum][i].send_summon_monster_packet(M);
			}
		}
		TIMER_EVENT ev{ roomNum, M->m_id, high_resolution_clock::now(), EV_RANDOM_MOVE};
		timer_queue.push(ev);
	}
}

void SESSION::CheckPosition(XMFLOAT3 newPos)
{
	// 이동속도가 말도 안되게 빠른 경우 체크
	static const float max_distance = 100.f;
	XMFLOAT3 Distance = Vector3::Subtract(newPos, GetPosition());
	float distance_squared = Distance.x * Distance.x + Distance.z * Distance.z;
	if (distance_squared > max_distance * max_distance) {
		cout << "client[" << _id << "] 에러 포인트 감지\n";
		m_xmf3Velocity = XMFLOAT3{ 0,0,0 };
		return;
	}
	
	else
	{
		try {
			for (const auto& object : Objects.at((int)newPos.z / AREA_SIZE)) {	// array의 멤버함수 at은 잘못된 인덱스로 접근하면 excetion을 호출한다
				if (object->m_xmOOBB.Contains(XMLoadFloat3(&newPos))) {
					m_xmf3Velocity = XMFLOAT3{ 0,0,0 };
					return;
				}
			}
		}
		catch (const exception& e) {
			cout << "위치 에러\n";
			disconnect(_id);
			return;
		}
	}


	SetPosition(newPos);
	UpdateBoundingBox();

	short stage = (short)((GetPosition().z ) / STAGE_SIZE);

	if (stage > cur_stage) {
		Initialize_Monster(_id / 4, stage);
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_ROOM; ++i) {
		for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
			if (clients[i][j]._state.load() == ST_FREE)
				return i * 4 + j;
		}
	}
	return -1;
}



void SESSION::Update()
{
	if (HP <= 0)
	{
		direction.store(DIR_DIE);
		_state.store(ST_DEAD);
		return;
	}
	float ElapsedTime = duration_cast<milliseconds>(high_resolution_clock::now() - recent_recvedTime).count() / 1000.f;

	BulletPos = Vector3::Add(BulletPos, Vector3::ScalarProduct(BulletLook, ElapsedTime * 100, false));
	for (auto& monster : PoolMonsters[_id / 4]) {
		lock_guard<mutex> mm{ monster->m_lock };
		if (monster->HP > 0 && BoundingBox(BulletPos, BULLET_SIZE).Intersects(monster->BB))
		{
			monster->HP -= 200;
			if (monster->HP <= 0)
				monster->SetState(NPC_State::Dead);

			BulletPos = XMFLOAT3(5000, 5000, 5000);
			break;
		}
	}
	recent_recvedTime = high_resolution_clock::now();
}


bool check_path(const XMFLOAT3& _pos, unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal>& CloseList, BoundingBox& check_box)
{
	auto start = clock();
	int collide_range = (int)(_pos.z / AREA_SIZE);
	check_box.Center = _pos;

	if (CloseList.find(_pos) != CloseList.end()) {
		return false;
	}
	try {
		if (Objects[collide_range].end() != find_if(Objects[collide_range].begin(), Objects[collide_range].end(),
			[check_box](MapObject* MO) {return MO->m_xmOOBB.Intersects(check_box); }))
		{
			return false;
		}
	}
	catch (const exception& e)
	{
		cout << "위치 에러\n";
		return false;
	}
	return true;
}

unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>::iterator getNode(unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>& m_List)
{
	return min_element(m_List.begin(), m_List.end(), 
		[](const pair<const XMFLOAT3, shared_ptr<A_star_Node>>& p1, const pair<const XMFLOAT3, shared_ptr<A_star_Node>>& p2) {return p1.second->F < p2.second->F; });
}

bool check_openList(XMFLOAT3& _Pos, float _G, shared_ptr<A_star_Node> s_node, unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>& m_List)
{
	auto start = clock();
	auto iter = m_List.find(_Pos);
	if (iter != m_List.end()) {
		if ((*iter).second->G > _G) {
			(*iter).second->G = _G;
			(*iter).second->F = (*iter).second->G + (*iter).second->H;
			(*iter).second->parent = s_node;
		}
		return false;
	}
	return true;
}

float nx[8]{ -1,1,0,0, -1, -1, 1, 1 };
float nz[8]{ 0,0,1,-1, -1, 1, -1, 1 };
XMFLOAT3 Monster::Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos)
{
	if (dest_Pos.y - start_Pos.y >= 2.f)
	{
		SetState(NPC_State::Idle);
		cur_animation_track = 0;
		target_id = -1;
		return Pos;
	}

	unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal> closelist{};
	priority_queue<shared_ptr<A_star_Node>, vector<shared_ptr<A_star_Node>>, CompareNodes> openlist;
	shared_ptr<A_star_Node> S_Node;
	check_pathTime = 0;
	check_openListTime = 0;


	//openlist.emplace(start_Pos, make_shared<A_star_Node>(start_Pos, dest_Pos));
	openlist.emplace(make_shared<A_star_Node>(start_Pos, dest_Pos));

	BoundingBox CheckBox = BoundingBox(start_Pos, BB.Extents);
	while (!openlist.empty())
	{
		S_Node = openlist.top();
		openlist.pop();

		if (clients[room_num][target_id].m_xmOOBB.Contains(XMLoadFloat3(&S_Node->Pos)))
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
			if (check_path(_Pos, closelist, CheckBox)) {
				openlist.emplace(make_shared<A_star_Node>(_Pos, dest_Pos, S_Node->G + speed * sqrt(abs(nx[i]) + abs(nz[i])), S_Node));
			}
		}
		closelist.insert(S_Node->Pos);
		//openlist.erase(iter);
	}
	cout << "Trace Failed\n";
	SetState(NPC_State::Idle);
	cur_animation_track = 0;
	target_id = -1;
	return Pos;
}

int Monster::get_targetID()
{
	for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
		if (clients[room_num][i]._state.load() != ST_INGAME ||
			clients[room_num][i].GetPosition().y - Pos.y >= 2.f) {
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

	const auto& targetPlayer = &clients[room_num][target_id];
	// 몬스터와 플레이어 사이의 벡터
	XMVECTOR distanceVector = DirectX::XMLoadFloat3(&targetPlayer->GetPosition()) - DirectX::XMLoadFloat3(&Pos);

	// 몬스터와 플레이어 사이의 거리 계산
	g_distance = XMVectorGetX(XMVector3Length(distanceVector));

	if (4 == type && MagicPos.x != 5000) {
		MagicPos = Vector3::Add(MagicPos, Vector3::ScalarProduct(MagicLook, 5, false));
		if (BoundingBox(MagicPos, BULLET_SIZE).Intersects(targetPlayer->m_xmOOBB))
		{
			lock_guard <mutex> ll{ targetPlayer->_s_lock };
			targetPlayer->HP -= GetPower();
			MagicPos.x = 5000;
			cout << "plHP : " << targetPlayer->HP << endl;
		}
	}
	switch (GetState())
	{
	case NPC_State::Idle: {
		target_id = get_targetID();
		if (target_id != -1) {
			SetState(NPC_State::Chase);
			cur_animation_track = 1;
		}
	}
		break;
	case NPC_State::Chase: {
		if (targetPlayer->_state != ST_INGAME || g_distance > view_range) {
			SetState(NPC_State::Idle);
			cur_animation_track = 0;
			target_id = -1;
			roadToMove = stack<XMFLOAT3>();
			break;
		}
		if ((4 == type && 150 >= g_distance) || 20 >= g_distance)
		{
			SetState(NPC_State::Attack);
			cur_animation_track = (type != 2) ? 2 : cur_animation_track;
			roadToMove = stack<XMFLOAT3>();
			break;
		}

		if (!roadToMove.empty()) 
		{
			Pos = roadToMove.top();
			roadToMove.pop();
		}
		else {
			const int collide_range = (int)(Pos.z / AREA_SIZE);
			const XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(Vector3::RemoveY(Vector3::Normalize(Vector3::Subtract(targetPlayer->GetPosition(), Pos))), speed, false));

			bool collide = false;
			for (const auto& obj : Objects[collide_range]) {
				if (obj->m_xmOOBB.Intersects(BoundingBox(newPos, BB.Extents))) {
					collide = true;
					break;
				}
			}
			if (collide) {
				Pos = Find_Direction(Pos, targetPlayer->GetPosition());
				BB.Center = Pos;
			}
			else {
				Pos = newPos;
				BB.Center = Pos;
			}
		}
	}
		break;
	case NPC_State::Attack: {
		attack_timer -= fTimeElapsed;
		if (targetPlayer->_state != ST_INGAME) {
			SetState(NPC_State::Idle);
			cur_animation_track = 0;
			target_id = -1;
			SetAttackTimer(attack_cycle);
			break;
		}
		if (attacked == false && GetAttackTimer() <= attack_cycle / 2.f) {
			if (4 == type && 150 > g_distance) {
				MagicPos = Vector3::Add(GetPosition(), XMFLOAT3(0, 10, 0));
				XMStoreFloat3(&MagicLook, XMVector3Normalize(distanceVector));
			}
			else if (20 > g_distance) {
				lock_guard <mutex> ll{ targetPlayer->_s_lock };
				targetPlayer->HP -= GetPower();
				cout << "plHP : " << targetPlayer->HP << endl;
			}
			attacked = true;
			break;
		}
		if (GetAttackTimer() <= 0) {//0326
			if (4 == type && 150 <= g_distance)
			{
				SetState(NPC_State::Chase);
				cur_animation_track = 1;
				SetAttackTimer(attack_cycle);
			}
			else if (20 <= g_distance) {//부딪히지x
				SetState(NPC_State::Chase);
				cur_animation_track = 1;
				SetAttackTimer(attack_cycle);
			}
			SetAttackTimer(attack_cycle);
			attacked = false;
		}
	}
		break;
	case NPC_State::Dead: {
		if (type != 2)
			cur_animation_track = 3;
		dead_timer -= fTimeElapsed;
		if (dead_timer <= 0) {
			SetAlive(false);
		}
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
		for (int i = 0; i < 5; ++i) {
			StagesInfo[0].push_back(MonsterInfo(XMFLOAT3(-27.f + rand() % 100, -292, 1460), 0, ID_constructor++));
		}
	}
	{	// 2stage
		for (int i = 0; i < 3; ++i) {
			StagesInfo[1].push_back(MonsterInfo(XMFLOAT3(-50.f, -292, 1500), 1, ID_constructor++));
		}
	}
	{	// 3stage
		for (int i = 0; i < 3; ++i) {
			StagesInfo[2].push_back(MonsterInfo(XMFLOAT3(-50.f, -292, 1900), 4, ID_constructor++));
		}
	}
	{	// 4stage
	}
	{	// 5stage

	}
	{	// 6stage

	}
}

