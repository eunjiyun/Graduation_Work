#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "MemoryPool.h"
#include "SESSION.h"
#include "Timer.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;




array<SESSION, MAX_USER> clients;

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
CGameTimer m_GameTimer;
MapObject** m_ppObjects = 0;
vector<MapObject*> Objects[6] = {};
int m_nObjects = 0;

void disconnect(int c_id)
{
	for (auto& pl : clients) {
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
	closesocket(clients[c_id]._socket);

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	clients[c_id]._state = ST_FREE;
}

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.Look = clients[c_id].GetLookVector();
	p.Right = clients[c_id].GetRightVector();
	p.Up = clients[c_id].GetUpVector();
	p.Pos = clients[c_id].GetPosition();
	p.direction = clients[c_id].direction;
	//clients[c_id].direction = 0;
	do_send(&p);

}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.Look = clients[c_id].m_xmf3Look;
	add_packet.Right = clients[c_id].m_xmf3Right;
	add_packet.Up = clients[c_id].m_xmf3Up;
	add_packet.Pos = clients[c_id].GetPosition();
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

			cout << "Name: " << object->m_pstrName << "\nCenter: " << oBox.Center.x << ", " << oBox.Center.y << ", " << oBox.Center.z <<
				"\nExtents: " << oBox.Extents.x << ", " << oBox.Extents.y << ", " << oBox.Extents.z << endl;

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
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		cout << "Client[" << c_id << "] Accessed\n";
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id]._s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			pl.send_add_player_packet(c_id);
			clients[c_id].send_add_player_packet(pl._id);
		}
		break;
	}
	case CS_MOVE: {
		lock_guard <mutex> ll{ clients[c_id]._s_lock };
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		
		clients[c_id].CheckPosition(p->pos);
		clients[c_id].direction = p->direction;
		clients[c_id].Rotate(p->cxDelta, p->cyDelta, p->czDelta);
	}
	}
}


void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].m_xmf3Position.x = -50;
				clients[client_id].m_xmf3Position.y = -20;
				clients[client_id].m_xmf3Position.z = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		}
	}
}

void update_thread()
{
	m_GameTimer.Start();
	while (1)
	{
		m_GameTimer.Tick(0.0f);
		for (int i = 0; i < 4; i++) {
			lock_guard <mutex> ll{ clients[i]._s_lock };
			if (clients[i]._state != ST_INGAME) continue;
			clients[i].Update(m_GameTimer.GetTimeElapsed());
			clients[i].CheckCollision(m_GameTimer.GetTimeElapsed());
			clients[i].Deceleration(m_GameTimer.GetTimeElapsed());
			for (auto& cl : clients) {
				cl.send_move_packet(clients[i]._id);
			}
		}
		Sleep(100);
	}
}

int main()
{
	m_ppObjects = LoadGameObjectsFromFile("Models/Scene.bin", &m_nObjects);
	for (int i = 0; i < m_nObjects; i++) {
		int collide_range_min = ((int)m_ppObjects[i]->m_xmOOBB.Center.z - (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		int collide_range_max = ((int)m_ppObjects[i]->m_xmOOBB.Center.z + (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		for (int j = collide_range_min; j <= collide_range_max; j++) {
			Objects[j].push_back(m_ppObjects[i]);
		}		
	}
	
	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector <thread> worker_threads;
	thread* update_t = new thread{ update_thread };
	//int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < 8; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	update_t->join();
	closesocket(g_s_socket);
	WSACleanup();
}
