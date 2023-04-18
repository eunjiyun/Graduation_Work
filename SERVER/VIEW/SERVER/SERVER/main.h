#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"


enum EVENT_TYPE { EV_RANDOM_MOVE };

//#define _GRID_MAP

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
array<threadsafe_vector<Monster*>, MAX_ROOM> PoolMonsters;


CObjectPool<Monster> MonsterPool(20'000);
array<vector<MonsterInfo>, 10> StagesInfo;

int check_pathTime = 0;
int check_openListTime = 0;

bool GridMap[2][MAP_X_SIZE][MAP_Z_SIZE];
array<vector< MapObject*>, OBJECT_ARRAY_SIZE> Objects;
array<Stage_Location_Info, STAGE_NUMBERS> StageLocations;

SESSION& getClient(int c_id)
{
	return clients[c_id / 4][c_id % 4];
}

array<SESSION, MAX_USER_PER_ROOM>& getRoom(int c_id)
{
	return clients[c_id / 4];
}

threadsafe_vector<Monster*>& getMonsters(int c_id)
{
	return PoolMonsters[c_id / 4];
}

vector<MapObject*>& getPartialObjects(XMFLOAT3 Pos)
{
	return Objects[(int)Pos.z / STAGE_SIZE];
}


void disconnect(int c_id)
{
	bool game_in_progress = false;
	auto& Monsters = getMonsters(c_id);
	for (auto& pl : getRoom(c_id)) {

		if ((ST_INGAME != pl._state.load() && ST_DEAD != pl._state.load()) || pl._id == c_id) continue;

		pl.send_remove_player_packet(c_id);
		game_in_progress = true;
	}
	SESSION& CL = getClient(c_id);
	closesocket(CL._socket);

	if (game_in_progress) // 아직 방 안에 사람이 있다면 
		CL._state.store(ST_CRASHED);

	else {// 방이 비었다면 몬스터와 플레이어 컨테이너 모두 정리
		for (auto& pl : getRoom(c_id)) {
			pl._state.store(ST_FREE);
		}
		unique_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
		for (auto iter = Monsters.begin(); iter != Monsters.end();) {
			lock_guard<mutex> mm{ (*iter)->m_lock };
			MonsterPool.ReturnMemory(*iter);
			Monsters.erase(iter);
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
	p.animation_track = M->cur_animation_track; // 원래 p.animation_track = (short)M->GetState();가 맞는데 2번 귀신이 애니메이션이 빠져 있어서 이렇게 함
	p.Chasing_PlayerID = M->target_id;
	p.BulletPos = M->MagicPos;
	p.room_num = M->room_num;
	do_send(&p);
}

void Initialize_Monster(int roomNum, int stageNum)//0326
{
	if (PoolMonsters[roomNum].size() > 0) return;

	for (auto& info : StagesInfo[stageNum - 1]) {
		Monster* M = MonsterPool.GetMemory();
		M->Initialize(roomNum, info.id, info.type, info.Pos);
		PoolMonsters[roomNum].emplace_back(M);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			if (clients[roomNum][i]._state.load() == ST_INGAME || clients[roomNum][i]._state.load() == ST_DEAD) {
				clients[roomNum][i].cur_stage.store(stageNum);
				clients[roomNum][i].send_summon_monster_packet(M);
			}
		}
		TIMER_EVENT ev{ roomNum, M->m_id, high_resolution_clock::now(), EV_RANDOM_MOVE};
		timer_queue.push(ev);
	}
}

void SESSION::CheckPosition(XMFLOAT3 newPos)
{
	// check velocity
	static const float max_distance = 100.f;
	XMFLOAT3 Distance = Vector3::Subtract(newPos, GetPosition());
	float distance_squared = Distance.x * Distance.x + Distance.z * Distance.z;
	if (distance_squared > max_distance * max_distance) {
		m_xmf3Velocity = XMFLOAT3{ 0,0,0 };
		return;
	}


	try {
		for (const auto& object : Objects.at((int)newPos.z / AREA_SIZE)) {	// array의 멤버함수 at은 잘못된 인덱스로 접근하면 exception을 호출
			if (object->m_xmOOBB.Contains(XMLoadFloat3(&newPos))) {
				m_xmf3Velocity = XMFLOAT3{ 0,0,0 };
				return;
			}
		}
	}
	catch (const exception& e) {
		cout << "checkPosition catched error -" << e.what() << endl;
		disconnect(_id);
		return;
	}


	lock_guard<mutex> player_lock{ _s_lock };
	SetPosition(newPos);
	UpdateBoundingBox();
	short stage = (short)((GetPosition().z) / STAGE_SIZE);


	if (stage > cur_stage.load()) {
		Initialize_Monster(_id / MAX_USER_PER_ROOM, stage);
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_ROOM; ++i) {
		for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
			if (clients[i][j]._state.load() == ST_FREE)
				return i * MAX_USER_PER_ROOM + j;
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

	if (Vector3::Length(BulletLook) > 0) {
		float ElapsedTime = duration_cast<milliseconds>(high_resolution_clock::now() - recent_recvedTime).count() / 1000.f;


		BulletPos = Vector3::Add(BulletPos, Vector3::ScalarProduct(BulletLook, ElapsedTime * 100, false));
		{
			auto& Monsters = getMonsters(_id);
			shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
			for (auto& monster : Monsters) {
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && BoundingBox(BulletPos, BULLET_SIZE).Intersects(monster->BB))
				{
					monster->HP -= 200;
					if (monster->HP <= 0)
						monster->SetState(NPC_State::Dead);

					BulletPos = XMFLOAT3(5000, 5000, 5000);
					BulletLook = XMFLOAT3(0, 0, 0);
					break;
				}
			}
		}
		recent_recvedTime = high_resolution_clock::now();
	}
}


bool Monster::check_path(const XMFLOAT3& _pos, unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal>& CloseList, BoundingBox& check_box)
{
	auto start = clock();
	int collide_range_z = (int)(_pos.z / AREA_SIZE);
	if (collide_range_z < 0 || collide_range_z > 42 || _pos.x < -360 || _pos.x > 120) {
		return false;
	}
	check_box.Center = _pos;

	if (CloseList.find(_pos) != CloseList.end()) {
		return false;
	}


	try {
		if (Objects[collide_range_z].end() != find_if(Objects[collide_range_z].begin(), Objects[collide_range_z].end(),
			[check_box](MapObject* MO) {return MO->m_xmOOBB.Intersects(check_box); }))
		{
			return false;
		}
	}
	catch (const exception& e)
	{
		cout << e.what() << endl;
		return false;
	}
	return true;
}

unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>::iterator getNode(unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>& m_List)
{
	try {
		return min_element(m_List.begin(), m_List.end(),
			[](const pair<const XMFLOAT3, shared_ptr<A_star_Node>>& p1, const pair<const XMFLOAT3, shared_ptr<A_star_Node>>& p2)
			{
				if (p1.second && p2.second)
					return p1.second->F < p2.second->F;
				else {
					return false;
				}
			});
	}
	catch (const exception& e) {
		cout << "openlist access error - " << e.what() << endl;
		return m_List.end();
	}
}

bool check_openList(XMFLOAT3& _Pos, float _G, shared_ptr<A_star_Node> s_node, unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal>& m_List)
{
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
XMFLOAT3 Monster::Find_Direction(float fTimeElapsed, XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos)
{
	if (dest_Pos.y - start_Pos.y >= 2.f)
	{
		SetState(NPC_State::Idle);
		cur_animation_track = 0;
		target_id = -1;
		return Pos;
	}

	unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal> closelist{};
	unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal> openlist;
	shared_ptr<A_star_Node> S_Node;
	check_pathTime = 0;
	check_openListTime = 0;

	openlist.emplace(start_Pos, shared_ptr<A_star_Node>(new A_star_Node(start_Pos, dest_Pos, 0, nullptr)));

	BoundingBox CheckBox = BoundingBox(start_Pos, BB.Extents);
	while (!openlist.empty())
	{
		auto iter = getNode(openlist);
		if (iter == openlist.end()) { cout << "GetNode ERROR\n"; break; }

		S_Node = (*iter).second;

		//if (clients[room_num][target_id].m_xmOOBB.Intersects(BoundingBox(S_Node->Pos, BB.Extents)))
		if (Vector3::Length(Vector3::Subtract(clients[room_num][target_id].GetPosition(), S_Node->Pos)) < 30)
		{
			while (S_Node->parent != nullptr)
			{
				if (Vector3::Compare(S_Node->parent->Pos, start_Pos))
				{
					return S_Node->Pos;
				}
				S_Node = S_Node->parent;
			}
		}
		for (int i = 0; i < 8; i++) {
			XMFLOAT3 _Pos = Vector3::Add(S_Node->Pos, Vector3::ScalarProduct(XMFLOAT3{ nx[i],0,nz[i] }, speed * fTimeElapsed, false));
			float _G = S_Node->G + speed * fTimeElapsed * sqrt(abs(nx[i]) + abs(nz[i]));
			if (check_path(_Pos, closelist, CheckBox) && check_openList(_Pos, _G, S_Node, openlist)) {
				openlist.emplace(_Pos, shared_ptr<A_star_Node>(new A_star_Node(_Pos, dest_Pos, _G, S_Node)));
			}
		}
		closelist.insert(S_Node->Pos);
		openlist.erase(iter);
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
	XMFLOAT3 distanceVector = Vector3::Subtract(targetPlayer->GetPosition(), Pos);

	// 몬스터와 플레이어 사이의 거리 계산
	g_distance = Vector3::Length(distanceVector);
	if (4 == type && MagicPos.x != 5000) {
		MagicPos = Vector3::Add(MagicPos, Vector3::ScalarProduct(MagicLook, 5, false));
		if (BoundingBox(MagicPos, BULLET_SIZE).Intersects(targetPlayer->m_xmOOBB))
		{
			lock_guard <mutex> ll{ targetPlayer->_s_lock };
			targetPlayer->HP -= GetPower();
			MagicPos.x = 5000;
			//cout << "plHP : " << targetPlayer->HP << endl;
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
			return;
		}
		if ((4 == type && LONGRANGETTACK_RANGE >= g_distance) || MELEEATTACK_RANGE >= g_distance)
		{
			SetState(NPC_State::Attack);
			cur_animation_track = (type != 2) ? 2 : cur_animation_track;
			return;
		}
		const int collide_range = (int)(Pos.z / AREA_SIZE);
		const XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(Vector3::Normalize(Vector3::Subtract(targetPlayer->GetPosition(), Pos)), speed * fTimeElapsed, false));

		bool collide = false;
		for (const auto& obj : Objects[collide_range]) {
			if (obj->m_xmOOBB.Intersects(BoundingBox(newPos, BB.Extents))) {
				collide = true;
				break;
			}
		}
		if (collide) {
			Pos = Find_Direction(fTimeElapsed, Pos, targetPlayer->GetPosition());
			BB.Center = Pos;
		}
		else {
			Pos = newPos;
			BB.Center = Pos;
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
			if (4 == type) {
				MagicPos = Vector3::Add(GetPosition(), XMFLOAT3(0, 10, 0));
				MagicLook = Vector3::Normalize(distanceVector);
			}
			else if (MELEEATTACK_RANGE > g_distance) {
				lock_guard <mutex> ll{ targetPlayer->_s_lock };
				targetPlayer->HP -= GetPower();
				//cout << "plHP : " << targetPlayer->HP << endl;
			}
			attacked = true;
			break;
		}
		if (GetAttackTimer() <= 0) {//0326
			if (4 == type && LONGRANGETTACK_RANGE <= g_distance)
			{
				SetState(NPC_State::Chase);
				cur_animation_track = 1;
				SetAttackTimer(attack_cycle);
			}
			else if (MELEEATTACK_RANGE <= g_distance) {
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
#ifdef _GRID_MAP
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> x_dis(130, 570);
	uniform_int_distribution<int> z_dis(1260, 2550);
	uniform_int_distribution<int> type_dis(0, 1);
	{	// 1stage
		cout << "1 stage\n";
		//StagesInfo[0].push_back(MonsterInfo(XMFLOAT3(-27.f + dis(gen), -300, 1460), 0, ID_constructor++));
		for (int i = 0; i < 10; i++) {
			int _x = x_dis(gen);
			int _z = z_dis(gen);
			if (GridMap[1][_x][_z] == false) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3((float)_x, -60, (float)_z), type_dis(gen), ID_constructor++);
			StagesInfo[0].push_back(MI);
			cout << ID_constructor << " - ";
			Vector3::Print(MI.Pos);
		}
	}
	{	// 2stage
		cout << "2 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(2650, 3550));
		for (int i = 0; i < 10; i++) {
			int _x = x_dis(gen);
			int _z = z_dis(gen);
			if (GridMap[1][_x][_z] == false) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3((float)_x, -60, (float)_z), type_dis(gen), ID_constructor++);
			StagesInfo[1].push_back(MI);
			cout << ID_constructor << " - ";
			Vector3::Print(MI.Pos);
		}
	}
	{	// 3stage
	}
	{	// 4stage
	}
	{	// 5stage
	}
	{	// 6stage
	}
#else

#endif
}

