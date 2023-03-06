#include "SESSION.h"
#include "MemoryPool.h"


array<array<SESSION, MAX_USER_PER_ROOM>, MAX_USER / MAX_USER_PER_ROOM> clients;
array<array<Monster, MAX_MONSTER_PER_ROOM>, MAX_USER / MAX_USER_PER_ROOM> monsters;

MapObject** m_ppObjects = 0;
vector<MapObject*> Objects[6] = {};
int m_nObjects = 0;

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
	closesocket(clients[c_id / 4][c_id % 4]._socket);

	lock_guard<mutex> ll(clients[c_id / 4][c_id % 4]._s_lock);
	clients[c_id / 4][c_id % 4]._state = ST_FREE;
}

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.Look = clients[c_id / 4][c_id % 4].GetLookVector();
	p.Right = clients[c_id / 4][c_id % 4].GetRightVector();
	p.Up = clients[c_id / 4][c_id % 4].GetUpVector();
	p.Pos = clients[c_id / 4][c_id % 4].GetPosition();
	p.direction = clients[c_id / 4][c_id % 4].direction;
	//p.move_time = clients[c_id / 4][c_id % 4]._last_move_time;
	do_send(&p);

}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id / 4][c_id % 4]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.Look = clients[c_id / 4][c_id % 4].m_xmf3Look;
	add_packet.Right = clients[c_id / 4][c_id % 4].m_xmf3Right;
	add_packet.Up = clients[c_id / 4][c_id % 4].m_xmf3Up;
	add_packet.Pos = clients[c_id / 4][c_id % 4].GetPosition();
	do_send(&add_packet);
}

void SESSION::send_summon_monster_packet(int npc_id)
{
	SC_SUMMON_MONSTER_PACKET summon_packet;
	summon_packet.id = npc_id;
	summon_packet.size = sizeof(summon_packet);
	summon_packet.type = SC_SUMMON_MONSTER;
	//summon_packet.Look = clients[c_id / 4][c_id % 4].m_xmf3Look;
	//summon_packet.Right = clients[c_id / 4][c_id % 4].m_xmf3Right;
	//summon_packet.Up = clients[c_id / 4][c_id % 4].m_xmf3Up;
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
	//p.Look = clients[c_id / 4][c_id % 4].GetLookVector();
	//p.Right = clients[c_id / 4][c_id % 4].GetRightVector();
	//p.Up = clients[c_id / 4][c_id % 4].GetUpVector();
	p.Pos = monsters[_id / 4][npc_id].GetPosition();
	//p.direction = clients[c_id / 4][c_id % 4].direction;
	//p.move_time = clients[c_id / 4][c_id % 4]._last_move_time;
	do_send(&p);
}
void SESSION::CheckPosition(XMFLOAT3 newPos)
{
	// 이동속도가 말도 안되게 빠른 경우 체크
	XMFLOAT3 Distance = Vector3::Subtract(newPos, GetPosition());
	if (sqrtf(Distance.x * Distance.x + Distance.z * Distance.z) > 100.f) {
		error_stack++;
		cout << "client[" << _id << "] 에러 포인트 감지\n";
	}

	SetPosition(newPos);
	UpdateBoundingBox();

	if (error_stack > 500) {
		disconnect(_id);
		cout << "에러 스택 500 초과 플레이어 추방\n";
	}

}

void SESSION::CheckCollision(float fTimeElapsed)
{
	XMFLOAT3 Vel = GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);

	int collide_range = (int)GetPosition().z / 600;

	for (MapObject*& object : Objects[collide_range]) {
		//for (int i = 0; i < m_nObjects; ++i) {
		BoundingBox oBox = object->m_xmOOBB;
		if (m_xmOOBB.Intersects(oBox)) {
			if (0 == strncmp(object->m_pstrName, "Dense_Floor_mesh", 16) || 0 == strncmp(object->m_pstrName, "Ceiling_base_mesh", 17)) {
				XMFLOAT3 Pos = GetPosition();
				Pos.y = oBox.Center.y + oBox.Extents.y + m_xmOOBB.Extents.y;
				SetPosition(Pos);
				continue;
			}

			/*cout << "Name: " << object->m_pstrName << "\nCenter: " << oBox.Center.x << ", " << oBox.Center.y << ", " << oBox.Center.z <<
				"\nExtents: " << oBox.Extents.x << ", " << oBox.Extents.y << ", " << oBox.Extents.z << endl;*/

			XMFLOAT3 ObjLook = { 0,0,0 };
			if (oBox.Center.x - oBox.Extents.x < m_xmOOBB.Center.x && oBox.Center.x + oBox.Extents.x > m_xmOOBB.Center.x) {
				if (oBox.Center.z < m_xmOOBB.Center.z) ObjLook = { 0,0,1 };
				else ObjLook = { 0, 0, -1 };
			}
			else if (oBox.Center.x < m_xmOOBB.Center.x) ObjLook = { 1,0,0 };
			else ObjLook = { -1, 0, 0 };

			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			Move(MovVec, false);
		}
	}
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
			//monsters[roomNum][i] = Monster(roomNum, 1, { -100.f + 50.f * i, -20.f, 600.f });
			monsters[roomNum][i].Initialize(roomNum, 1, { -100.f + 50.f * i, -20.f, 600.f });
		}
		break;
	case 2:
		monster_count = 3;
		for (int i = 0; i < monster_count; ++i) {
			monsters[roomNum][i].Initialize(roomNum, 2, { -50.f + 50.f * i, -20.f, 1200.f });
		}
		break;
	case 3:
		monster_count = 4;
		for (int i = 0; i < monster_count; ++i) {
			monsters[roomNum][i].Initialize(roomNum, 3, { -170.f + 50.f * i, -20.f, 1800.f });
		}
		break;
	}
	return monster_count;
}

void SESSION::Update(float fTimeElapsed)
{

	Move(direction, 21.0f, true);

	if (onAttack || onCollect || onDie) m_xmf3Velocity = { 0, 0, 0 };

	if (onAttack)
	{
		for (auto& monster : monsters[_id/4])
			if (BoundingBox(GetPosition(), { 3,0,3 }).Intersects(monster.BB))
			{
				// monster.HP -= power; 
			}
	}
	if (onCollect)
	{

	}
	if (onDie)
	{
		// 사망 애니메이션 출력  
	}

	if (onRun) m_fMaxVelocityXZ = 100.0f; else m_fMaxVelocityXZ = 10.0f;



	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}



	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	CheckCollision(fTimeElapsed);
	Deceleration(fTimeElapsed);

	short stage = (GetPosition().z + 400) / 600;
	if (stage > cur_stage) {
		int monster_count = Initialize_Monster(_id / 4, stage);
		for (int i = 0; i < MAX_USER_PER_ROOM; ++i) {
			for (int j = 0; j < monster_count; ++j) {
				clients[_id / 4][i].send_summon_monster_packet(j);
			}
			clients[_id / 4][i].cur_stage = stage;
			cout << _id / 4 << "번 방 " << stage << " 스테이지 몬스터 소환\n";
		}
	}

}

void check_queue(priority_queue<A_star_Node*>& queue, XMFLOAT3 enqueue_pos, int _G)
{
	priority_queue<A_star_Node*> copy_queue = queue;
	while (!copy_queue.empty())
	{
		A_star_Node* P = copy_queue.top();
		if (Vector3::Compare(P->Pos, enqueue_pos) && P->G > _G) {
			P->G = _G;
			P->F = P->G + P->H;
		}
	}
}
bool check_path(XMFLOAT3 _pos, vector<XMFLOAT3> CloseList)
{
	for (const auto& pos : CloseList) {
		if (pos.x == _pos.x && pos.z == _pos.z) // 이미 closedList에 있는 좌표면 
		{
			return false;
		}
	}

	int collide_range = _pos.z / 600;
	for (MapObject*& object : Objects[collide_range]) {
		if (0 == strncmp(object->m_pstrName, "Dense_Floor_mesh", 16) || 0 == strncmp(object->m_pstrName, "Ceiling_base_mesh", 17))
			continue;
		if (BoundingBox(_pos, { 5,3,5 }).Intersects(object->m_xmOOBB)) {
			return false;
		}
	}
	return true;
}

float nx[8]{ -1,1,0,0,-1,-1,1,1 };
float nz[8]{ 0,0,1,-1,1,-1,1,-1 };
XMFLOAT3 Monster::Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos)
{
	priority_queue<A_star_Node*, vector<A_star_Node*>, Comp> openList;
	vector<XMFLOAT3> CloseList{};
	//array<array<bool, 340>, 1000> check{ false };
	openList.push(new A_star_Node(start_Pos, dest_Pos));
	//check[start_Pos.x][start_Pos.z] = true;
	while (!openList.empty())
	{
		A_star_Node* node = openList.top();
		openList.pop();
		if (BoundingBox(node->Pos, { 5,3,5 }).Intersects(clients[room_num][target_id].m_xmOOBB))
		{
			while (node->parent != nullptr)
			{
				if (Vector3::Compare(node->parent->Pos, start_Pos))
				{
					return node->Pos;
				}
				node = node->parent;
			}
		}
		for (int i = 0; i < 4; i++) {
			XMFLOAT3 _Pos = Vector3::Add(node->Pos, Vector3::ScalarProduct(XMFLOAT3{ nx[i],0,nz[i] }, speed, false));
			if (check_path(_Pos, CloseList)) {
				openList.push(new A_star_Node(_Pos, dest_Pos, node->G + speed * sqrt(abs(nx[i]) + abs(nz[i])), node));
			}
		}
		CloseList.push_back(node->Pos);

		//openList.erase(openList.begin());
		//openList.begin()++;
	}
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

void Monster::Update()
{
	if (target_id < 0) {
		target_id = get_targetID();
		return;
	}
	if (BB.Intersects(clients[room_num][target_id].m_xmOOBB))
	{
		// attack
		return;
	}
	Pos = Find_Direction(Pos, clients[room_num][target_id].GetPosition());
	BB.Center = Pos;

}

