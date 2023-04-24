#pragma once

#include "SESSION.h"
#include "MemoryPool.h"
#include "MapObject.h"
#include "Monster.h"


enum EVENT_TYPE { EV_MOVE, EV_SUMMON };

//#define _GRID_MAP 

struct TIMER_EVENT {
	int room_id;
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

#ifdef _GRID_MAP
bool GridMap[2][MAP_X_SIZE][MAP_Z_SIZE];
#endif
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
	return Objects[static_cast<int>(Pos.z) / STAGE_SIZE];
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

	if (game_in_progress) // ���� �� �ȿ� ����� �ִٸ� 
		CL._state.store(ST_CRASHED);

	else {// ���� ����ٸ� ���Ϳ� �÷��̾� �����̳� ��� ����
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
	p.animation_track = M->cur_animation_track; // ���� p.animation_track = (short)M->GetState();�� �´µ� 2�� �ͽ��� �ִϸ��̼��� ���� �־ �̷��� ��
	//p.Chasing_PlayerID = M->target_id;
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
	}
	TIMER_EVENT ev{ roomNum, high_resolution_clock::now(), EV_MOVE };
	timer_queue.push(ev);
	if (PoolMonsters[roomNum].size() > 10) cout << roomNum << "���� �����ȯ�� �Ǿ����\n";
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
		for (const auto& object : Objects.at(static_cast<int>(newPos.z) / AREA_SIZE)) {	// array�� ����Լ� at�� �߸��� �ε����� �����ϸ� exception�� ȣ��
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
	short stage = 0;
	if (GetPosition().y >= -100.f)
		stage = static_cast<short>((GetPosition().z - 300.f) / STAGE_SIZE);
	else
		stage = 3 + static_cast<short>(MAP_Z_SIZE * 0.75f / GetPosition().z );

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
	//if (HP <= 0)
	//{
	//	direction.store(DIR_DIE);
	//	_state.store(ST_DEAD);
	//	return;
	//}

	//if (Vector3::Length(BulletLook) > 0) {
	//	float ElapsedTime = duration_cast<milliseconds>(high_resolution_clock::now() - recent_recvedTime).count() / 1000.f;


	//	BulletPos = Vector3::Add(BulletPos, Vector3::ScalarProduct(BulletLook, ElapsedTime * 100, false));

	//	for (const auto& obj : Objects[static_cast<int>(BulletPos.z) / AREA_SIZE]) {
	//		if (obj->m_xmOOBB.Contains(XMLoadFloat3(&BulletPos))) {
	//			BulletPos.y += 5000;
	//			BulletLook = XMFLOAT3(0, 0, 0);
	//			return;
	//		}
	//	}
	//	auto& Monsters = getMonsters(_id);
	//	shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
	//	for (auto& monster : Monsters) {
	//		lock_guard<mutex> mm{ monster->m_lock };
	//		if (monster->HP > 0 && BoundingBox(BulletPos, BULLET_SIZE).Intersects(monster->BB))
	//		{
	//			monster->HP -= 200;
	//			monster->target_id = _id;
	//			if (monster->HP <= 0)
	//				monster->SetState(NPC_State::Dead);

	//			BulletPos.y += 5000;
	//			BulletLook = XMFLOAT3(0, 0, 0);
	//			break;
	//		}
	//	}

	//	recent_recvedTime = high_resolution_clock::now();
	//}
}


bool Monster::check_path(const XMFLOAT3& _pos, unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal>& CloseList, BoundingBox& check_box)
{
	int collide_range_z = static_cast<int>(_pos.z / AREA_SIZE);
	check_box.Center = _pos;

	if (CloseList.find(_pos) != CloseList.end()) {
		return false;
	}

	if (_pos.x < 0 || _pos.z < 0 || _pos.x > MAP_X_SIZE || _pos.z > MAP_Z_SIZE) return false;

#ifdef _GRID_MAP
	int _x = static_cast<int>(_pos.x);
	int _z = static_cast<int>(_pos.z);
	if (_pos.y < -100) {
		if (GridMap[0][_x][_z] == false) return false;
	}
	else {
		if (GridMap[1][_x][_z] == false) return false;
	}
#else
	if (Objects[collide_range_z].end() != find_if(Objects[collide_range_z].begin(), Objects[collide_range_z].end(),
		[check_box](MapObject* MO) {return MO->m_xmOOBB.Intersects(check_box); }))
	{
		return false;
	}
#endif

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
	unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal> closelist{};
	unordered_map<XMFLOAT3, shared_ptr<A_star_Node>, XMFLOAT3Hash, XMFLOAT3Equal> openlist;
	openlist.reserve(200);
	closelist.reserve(600);
	shared_ptr<A_star_Node> S_Node;

	openlist.emplace(start_Pos, make_shared<A_star_Node>(start_Pos, dest_Pos, 0, nullptr));
	
	BoundingBox CheckBox = BoundingBox(start_Pos, BB.Extents);
	while (!openlist.empty())
	{
		auto iter = getNode(openlist);

		S_Node = (*iter).second;

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
				openlist.emplace(_Pos, make_shared<A_star_Node>(_Pos, dest_Pos, _G, S_Node));
			}
		}
		closelist.emplace(S_Node->Pos);
		openlist.erase(iter);
	}
	//cout << "Trace Failed\n";
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
	if (2 == type && MagicPos.x < 5000) {
		MagicPos = Vector3::Add(MagicPos, Vector3::ScalarProduct(MagicLook, 10, false));
		for (auto& player : clients[room_num]) {
			lock_guard <mutex> ll{ player._s_lock };
			if (BoundingBox(MagicPos, BULLET_SIZE).Intersects(player.m_xmOOBB))
			{
				player.HP -= GetPower();
				MagicPos.x = 5000;
				//cout << "plHP : " << player.HP << endl;
				if (player.HP <= 0)
				{
					//player.direction.store(DIR_DIE);
					player._state.store(ST_DEAD);
					for (auto& cl : clients[room_num]) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)
							cl.send_move_packet(&player);
					}
				}
			}
		}
		for (auto& obj : Objects[static_cast<int>(MagicPos.z) / AREA_SIZE]) {
			if (obj->m_xmOOBB.Contains(XMLoadFloat3(&MagicPos)))
				MagicPos.x = 5000;
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
		const auto& targetPlayer = &clients[room_num][target_id];
		XMFLOAT3 distanceVector = Vector3::Subtract(targetPlayer->GetPosition(), Pos);
		g_distance = Vector3::Length(distanceVector);

		if (clients[room_num][target_id].GetPosition().y - Pos.y >= 2.f || clients[room_num][target_id]._state.load() != ST_INGAME || g_distance > view_range)
		{
			SetState(NPC_State::Idle);
			cur_animation_track = 0;
			target_id = -1;
			return;
		}
		if ((2 == type && LONGRANGETTACK_RANGE >= g_distance) || MELEEATTACK_RANGE >= g_distance)
		{
			SetState(NPC_State::Attack);
			cur_animation_track = (type != 4) ? 2 : cur_animation_track;
			return;
		}
		const int collide_range = static_cast<int>(Pos.z / AREA_SIZE);
		XMFLOAT3 vel = Vector3::Normalize(Vector3::Subtract(targetPlayer->GetPosition(), Pos));
		vel.y = 0.f;
		const XMFLOAT3 newPos = Vector3::Add(Pos, Vector3::ScalarProduct(vel, speed * fTimeElapsed, false));

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
		const auto& targetPlayer = &clients[room_num][target_id];
		XMFLOAT3 distanceVector = Vector3::Subtract(targetPlayer->GetPosition(), Pos);
		g_distance = Vector3::Length(distanceVector);

		attack_timer -= fTimeElapsed;
		if (targetPlayer->_state != ST_INGAME) {
			SetState(NPC_State::Idle);
			cur_animation_track = 0;
			target_id = -1;
			SetAttackTimer(attack_cycle);
			break;
		}
		if (attacked == false && GetAttackTimer() <= attack_cycle / 2.f) {
			if (2 == type) {
				MagicPos = Vector3::Add(GetPosition(), XMFLOAT3(0, 10, 0));
				MagicLook = Vector3::Normalize(distanceVector);
			}
			else if (MELEEATTACK_RANGE > g_distance) {
				lock_guard <mutex> ll{ targetPlayer->_s_lock };
				targetPlayer->HP -= GetPower();
				//cout << "plHP : " << targetPlayer->HP << endl;
				if (targetPlayer->HP <= 0) {

					//player.direction.store(DIR_DIE);
					targetPlayer->_state.store(ST_DEAD);
					for (auto& cl : clients[room_num]) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD) 
							cl.send_move_packet(targetPlayer);
					}
				}
			}
			attacked = true;
			break;
		}
		if (GetAttackTimer() <= 0) {//0326
			if (2 == type && LONGRANGETTACK_RANGE <= g_distance)
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
		if (type != 4)
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
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> x_dis(130, 570);
	uniform_int_distribution<int> z_dis(1260, 2550);
	uniform_int_distribution<int> type_dis(0, 1);
#ifdef _GRID_MAP
	{	// 1stage
		cout << "1 stage\n";
		//StagesInfo[0].push_back(MonsterInfo(XMFLOAT3(-27.f + dis(gen), -300, 1460), 0, ID_constructor++));
		while (ID_constructor < 10) {
			int _x = x_dis(gen);
			int _z = z_dis(gen);
			if (GridMap[0][_x][_z] == false) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3((float)_x, -300, (float)_z), type_dis(gen), ID_constructor);
			StagesInfo[0].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}
	{	// 2stage
		cout << "2 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(2650, 3550));
		while (ID_constructor < 20) {
			int _x = x_dis(gen);
			int _z = z_dis(gen);
			if (GridMap[0][_x][_z] == false) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3((float)_x, -300, (float)_z), type_dis(gen), ID_constructor);
			StagesInfo[1].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
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
	{	// 1stage
		cout << "1 stage\n";
		int _type = 1;
		while (ID_constructor < 10) {
			float _x = static_cast<float>(x_dis(gen));
			float _z = static_cast<float>(z_dis(gen));
			BoundingBox test = BoundingBox(XMFLOAT3(_x, -60, _z), XMFLOAT3(15, 20, 12));
			bool col = false;
			for (auto& obj : Objects[static_cast<int>(_z) / AREA_SIZE])
				if (obj->m_xmOOBB.Intersects(test)) {
					col = true;
					break;
				}
			if (col) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3(_x, -60, _z), _type, ID_constructor);
			StagesInfo[0].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}
	{	// 2stage
		cout << "2 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(2650, 3550));
		while (ID_constructor < 20) {
			float _x = static_cast<float>(x_dis(gen));
			float _z = static_cast<float>(z_dis(gen));
			BoundingBox test = BoundingBox(XMFLOAT3(_x, -60, _z), XMFLOAT3(15, 20, 12));
			bool col = false;
			for (auto& obj : Objects[static_cast<int>(_z) / AREA_SIZE])
				if (obj->m_xmOOBB.Intersects(test)) {
					col = true;
					break;
				}
			if (col) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3(_x, -60, _z), type_dis(gen), ID_constructor);
			StagesInfo[1].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}
	{	// 3stage
		cout << "3 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(3700, 4400));

		while (ID_constructor < 30) {
			float _x = static_cast<float>(x_dis(gen));
			float _z = static_cast<float>(z_dis(gen));
			BoundingBox test = BoundingBox(XMFLOAT3(_x, -300, _z), XMFLOAT3(15, 20, 12));
			bool col = false;
			for (auto& obj : Objects[static_cast<int>(_z) / AREA_SIZE])
				if (obj->m_xmOOBB.Intersects(test)) {
					col = true;
					break;
				}
			if (col) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3(_x, -300, _z), type_dis(gen), ID_constructor);
			StagesInfo[2].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}
	{	// 4stage
		cout << "4 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(2650, 3550));
		type_dis.param(uniform_int_distribution<int>::param_type(0, 2));
		while (ID_constructor < 40) {
			float _x = static_cast<float>(x_dis(gen));
			float _z = static_cast<float>(z_dis(gen));
			BoundingBox test = BoundingBox(XMFLOAT3(_x, -300, _z), XMFLOAT3(15, 20, 12));
			bool col = false;
			for (auto& obj : Objects[static_cast<int>(_z) / AREA_SIZE])
				if (obj->m_xmOOBB.Intersects(test)) {
					col = true;
					break;
				}
			if (col) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3(_x, -300, _z), type_dis(gen), ID_constructor);
			StagesInfo[3].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}
	{	// 5stage
		cout << "5 stage\n";
		gen.seed(rd());
		z_dis.param(uniform_int_distribution<int>::param_type(1260, 2550));
		while (ID_constructor < 50) {
			float _x = static_cast<float>(x_dis(gen));
			float _z = static_cast<float>(z_dis(gen));
			BoundingBox test = BoundingBox(XMFLOAT3(_x, -300, _z), XMFLOAT3(15, 20, 12));
			bool col = false;
			for (auto& obj : Objects[static_cast<int>(_z) / AREA_SIZE])
				if (obj->m_xmOOBB.Intersects(test)) {
					col = true;
					break;
				}
			if (col) continue;
			MonsterInfo MI = MonsterInfo(XMFLOAT3(_x, -300, _z), type_dis(gen), ID_constructor);
			StagesInfo[4].push_back(MI);
			cout << ID_constructor << " - " << MI.type << endl;
			Vector3::Print(MI.Pos);
			ID_constructor++;
		}
	}

	{	// 6stage
	}
#endif
}

