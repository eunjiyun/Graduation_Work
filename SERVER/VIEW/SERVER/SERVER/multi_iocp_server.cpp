#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "main.h"
#include "Timer.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
CGameTimer m_GameTimer;
HANDLE h_iocp;

void process_packet(int c_id, char* packet)
{
	SESSION* CL = getClient(c_id);
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(CL->_name, p->name);
		CL->send_login_info_packet();
		{
			lock_guard<mutex> ll{ CL->_s_lock };
			CL->_state = ST_INGAME;
			CL->direction = 0;
			CL->m_xmf3Up = XMFLOAT3{ 0,1,0 };
			CL->m_xmf3Right = XMFLOAT3{ 1,0,0 };
			CL->m_xmf3Look = XMFLOAT3{ 0,0,1 };
		}
		for (auto& pl : clients[c_id / 4]) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			pl.send_add_player_packet(&clients[c_id / 4][c_id % 4]);
			CL->send_add_player_packet(&pl);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		lock_guard <mutex> ll{ CL->_s_lock };
		CL->CheckPosition(p->pos);
		CL->direction = p->direction;
		CL->Rotate(p->cxDelta, p->cyDelta, p->czDelta);
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
			SESSION* CL = getClient(client_id);
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(CL->_s_lock);
					CL->_state = ST_ALLOC;
				}
				CL->m_xmf3Position.x = -50;
				CL->m_xmf3Position.y = 0;
				CL->m_xmf3Position.z = 1990;
				CL->_id = client_id;
				CL->_name[0] = 0;
				CL->_prev_remain = 0;
				CL->_socket = g_c_socket;
				CL->cur_stage = 0;
				CL->error_stack = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				CL->do_recv();
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
			SESSION* CL = getClient(key);
			int remain_data = num_bytes + CL->_prev_remain;
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
			CL->_prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			CL->do_recv();
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
	while (1)
	{
		for (int i = 0; i < MAX_USER / MAX_USER_PER_ROOM; ++i) {
			for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
				if (clients[i][j]._state != ST_INGAME) continue;
				{
					lock_guard <mutex> ll{ clients[i][j]._s_lock };
					clients[i][j].Update();
					if (clients[i][j].direction == DIR_DIE) clients[i][j]._state = ST_DEAD;
				}
				for (auto& cl : clients[i]) {
					if (cl._state == ST_INGAME || cl._state == ST_DEAD)  cl.send_move_packet(&clients[i][j]);
				}
			}
		}
		this_thread::sleep_for(100ms); // 0.1초당 한번 패킷 전달
	}
}

void update_NPC()
{
	m_GameTimer.Start();
	while (1)
	{
		m_GameTimer.Tick(30.0f);
		for (int i = 0; i < MAX_ROOM; ++i) {
			auto iter = PoolMonsters[i].begin();
			while (iter != PoolMonsters[i].end()) {	// iter erase를 위해 while 사용
				{
					lock_guard<mutex> mm{ (*iter).m_lock };
					(*iter).Update(m_GameTimer.GetTimeElapsed());
				}
				for (auto& cl : clients[i]) {
					if (cl._state == ST_INGAME || cl._state == ST_DEAD) cl.send_NPCUpdate_packet(&(*iter));
				}
				if ((*iter).HP <= 0) {
					MonsterPool.ReturnMemory(&(*iter));
					PoolMonsters[i].erase(iter);
					MonsterPool.PrintSize();
				}
				else
					iter++;
			}
		}
	}
	this_thread::sleep_for(30ms);
}

int main()
{
	m_ppObjects = LoadGameObjectsFromFile("Models/Scene.bin", &m_nObjects);
	for (int i = 0; i < m_nObjects; i++) {
		if (0 == strncmp(m_ppObjects[i]->m_pstrName, "Dense_Floor_mesh", 16) || 0 == strncmp(m_ppObjects[i]->m_pstrName, "Ceiling_base_mesh", 17)
			|| 0 == strncmp(m_ppObjects[i]->m_pstrName, "Stair_step", 10))
			continue;
		int collide_range_min = ((int)m_ppObjects[i]->m_xmOOBB.Center.z - (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		int collide_range_max = ((int)m_ppObjects[i]->m_xmOOBB.Center.z + (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		for (int j = collide_range_min; j <= collide_range_max; j++) {
			Objects[j].push_back(m_ppObjects[i]);
		}		
	}
	delete m_ppObjects;

	InitializeStages();


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
	thread* update_player_t = new thread{ update_thread };
	thread* update_NPC_t = new thread{ update_NPC };
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads - 2; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	update_player_t->join();
	update_NPC_t->join();
	closesocket(g_s_socket);
	WSACleanup();
}
