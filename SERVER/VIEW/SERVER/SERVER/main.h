#include "SESSION.h"



array<array<SESSION, MAX_USER_PER_ROOM>, MAX_USER / MAX_USER_PER_ROOM> clients;

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
	p.Look = clients[c_id/4][c_id%4].GetLookVector();
	p.Right = clients[c_id/4][c_id%4].GetRightVector();
	p.Up = clients[c_id/4][c_id%4].GetUpVector();
	p.Pos = clients[c_id/4][c_id%4].GetPosition();
	p.direction = clients[c_id/4][c_id%4].direction;
	//p.move_time = clients[c_id / 4][c_id % 4]._last_move_time;
	//clients[c_id/4][c_id%4].direction = 0;
	do_send(&p);

}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id/4][c_id%4]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.Look = clients[c_id/4][c_id%4].m_xmf3Look;
	add_packet.Right = clients[c_id/4][c_id%4].m_xmf3Right;
	add_packet.Up = clients[c_id/4][c_id%4].m_xmf3Up;
	add_packet.Pos = clients[c_id/4][c_id%4].GetPosition();
	do_send(&add_packet);
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

