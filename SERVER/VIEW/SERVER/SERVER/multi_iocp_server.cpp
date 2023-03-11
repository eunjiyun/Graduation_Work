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
	switch (packet[1]) {
	case CS_LOGIN: {
		//cout << "Client[" << c_id << "] Accessed\n";
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id / 4][c_id % 4]._name, p->name);
		clients[c_id / 4][c_id % 4].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id / 4][c_id % 4]._s_lock };
			clients[c_id / 4][c_id % 4]._state = ST_INGAME;
		}
		for (auto& pl : clients[c_id / 4]) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			pl.send_add_player_packet(c_id);
			clients[c_id / 4][c_id % 4].send_add_player_packet(pl._id);
		}
		break;
	}
	case CS_MOVE: {
		lock_guard <mutex> ll{ clients[c_id / 4][c_id % 4]._s_lock };
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id / 4][c_id % 4].CheckPosition(p->pos);
		clients[c_id / 4][c_id % 4].direction = p->direction;
		clients[c_id / 4][c_id % 4].Rotate(p->cxDelta, p->cyDelta, p->czDelta);
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
					lock_guard<mutex> ll(clients[client_id / 4][client_id % 4]._s_lock);
					clients[client_id / 4][client_id % 4]._state = ST_ALLOC;
				}
				clients[client_id / 4][client_id % 4].m_xmf3Position.x = -50;
				clients[client_id / 4][client_id % 4].m_xmf3Position.y = -17.5;
				clients[client_id / 4][client_id % 4].m_xmf3Position.z = 590;
				clients[client_id / 4][client_id % 4]._id = client_id;
				clients[client_id / 4][client_id % 4]._name[0] = 0;
				clients[client_id / 4][client_id % 4]._prev_remain = 0;
				clients[client_id / 4][client_id % 4]._socket = g_c_socket;
				clients[client_id / 4][client_id % 4].cur_stage = 0;
				clients[client_id / 4][client_id % 4].error_stack = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id / 4][client_id % 4].do_recv();
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
			int remain_data = num_bytes + clients[key / 4][key % 4]._prev_remain;
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
			clients[key / 4][key % 4]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key / 4][key % 4].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		//case OP_NPC_MOVE:
		//	bool keep_alive = false;
		//	for (int j = 0; j < MAX_USER; ++j)
		//		if (can_see(static_cast<int>(key), j)) {
		//			keep_alive = true;
		//			break;
		//		}
		//	if (true == keep_alive) {
		//		do_npc_random_move(static_cast<int>(key));
		//		TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
		//		timer_queue.push(ev);
		//	}
		//	else {
		//		clients[key]._is_active = false;
		//	}
		//	delete ex_over;
		//	break;
		}
	}
}

void update_thread()
{
	m_GameTimer.Start();
	while (1)
	{
		m_GameTimer.Tick(0.0f);
		for (int i = 0; i < MAX_USER / MAX_USER_PER_ROOM; ++i) {
			for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
				if (clients[i][j]._state != ST_INGAME) continue;
				{
					lock_guard <mutex> ll{ clients[i][j]._s_lock };
					clients[i][j].Update(m_GameTimer.GetTimeElapsed());
				}
				for (auto& cl : clients[i]) {
					cl.send_move_packet(clients[i][j]._id);
				}
			}
		}
		this_thread::sleep_for(100ms); // busy waiting을 막기 위해 잠깐 기다리는 함수
	}
}

void update_NPC()
{
	while (1)
	{
		//clock_t start_time = clock();
		for (int i = 0; i < MAX_USER / MAX_USER_PER_ROOM; ++i) {
			for (int k = 0; k < MAX_MONSTER_PER_ROOM; ++k) {
				if (monsters[i][k].is_alive) {
					monsters[i][k].Update();
					for (auto& cl : clients[i]) {
						cl.send_NPCUpdate_packet(k);
						this_thread::sleep_for(1ms); // busy waiting을 막기 위해 잠깐 기다리는 함수
					}
				}
			}
		}
		//cout << "1cycle - " << clock() - start_time << endl;
		this_thread::sleep_for(100ms); // busy waiting을 막기 위해 잠깐 기다리는 함수
	}
}

//void do_timer()
//{
//	while (true) {
//		TIMER_EVENT ev;
//		auto current_time = chrono::system_clock::now();
//		if (true == timer_queue.try_pop(ev)) {
//			if (ev.wakeup_time > current_time) {
//				timer_queue.push(ev);		// 최적화 필요
//				// timer_queue에 다시 넣지 않고 처리해야 한다.
//				this_thread::sleep_for(1ms);
//				continue;
//			}
//			switch (ev.event_id) {
//			case EV_RANDOM_MOVE:
//				OVER_EXP* ov = new OVER_EXP;
//				ov->_comp_type = OP_NPC_MOVE;
//				PostQueuedCompletionStatus(h_iocp, 1, ev.obj_id, &ov->_over);
//				break;
//			}
//		}
//		else this_thread::sleep_for(1ms);
//	}
//}


int main()
{
	m_ppObjects = LoadGameObjectsFromFile("Models/Scene.bin", &m_nObjects);
	clock_t start_time = clock();
	for (int i = 0; i < m_nObjects; i++) {
		//if (0 == strncmp(m_ppObjects[i]->m_pstrName, "Dense_Floor_mesh", 16) || 0 == strncmp(m_ppObjects[i]->m_pstrName, "Ceiling_base_mesh", 17))
		//	continue;
		int collide_range_min = ((int)m_ppObjects[i]->m_xmOOBB.Center.z - (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		int collide_range_max = ((int)m_ppObjects[i]->m_xmOOBB.Center.z + (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / 600;
		for (int j = collide_range_min; j <= collide_range_max; j++) {
			Objects[j].push_back(m_ppObjects[i]);
		}		
	}
	delete m_ppObjects;


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
