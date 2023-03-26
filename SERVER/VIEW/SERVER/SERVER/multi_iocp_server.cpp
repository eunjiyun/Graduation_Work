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
using namespace chrono;

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
CGameTimer m_NPCTimer;
CGameTimer m_PlayerTimer;
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
		}
		for (auto& pl : clients[c_id / 4]) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			pl.send_add_player_packet(CL);
			CL->send_add_player_packet(&pl);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		{
			lock_guard <mutex> ll{ CL->_s_lock };
			CL->CheckPosition(p->pos);
			CL->direction = p->direction;
			CL->Rotate(p->cxDelta, p->cyDelta, p->czDelta);
			CL->Update();
			if (CL->direction == DIR_DIE) CL->_state = ST_DEAD;
		}
		for (auto& cl : clients[c_id / 4]) {
			if (cl._state == ST_INGAME || cl._state == ST_DEAD)  cl.send_move_packet(CL);
		}
		break;
	}
	case CS_ATTACK: {
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		short type;
		{
			lock_guard<mutex> ll{ CL->_s_lock };
			type = CL->character_num;
		}
		switch (type)
		{
		case 0:
			for (auto& monster : PoolMonsters[CL->_id / 4]) {
				if (monster->HP > 0 && BoundingBox(p->pos, { 15,1,15 }).Intersects(monster->BB))
				{
					lock_guard<mutex> mm{ monster->m_lock };
					monster->HP -= 100;
				}
			}
			break;
		case 1:
			CL->BulletPos = Vector3::Add(CL->GetPosition(), XMFLOAT3(0, 10, 0));
			CL->BulletLook = CL->GetLookVector();
			break;
		case 2:
			for (auto& monster : PoolMonsters[CL->_id / 4]) {
				if (monster->HP > 0 && BoundingBox(p->pos, { 5,1,5 }).Intersects(monster->BB))
				{
					lock_guard<mutex> mm{ monster->m_lock };
					monster->HP -= 50;
				}
			}
			break;
		}
		for (auto& cl : clients[c_id / 4]) {
			if (cl._state == ST_INGAME || cl._state == ST_DEAD)  cl.send_attack_packet(CL);
		}
		break;
	}
	case CS_COLLECT: {
		CS_COLLECT_PACKET*p = reinterpret_cast<CS_COLLECT_PACKET*>(packet);
		for (auto& cl : clients[c_id / 4]) {
			if (cl._state == ST_INGAME || cl._state == ST_DEAD)  cl.send_collect_packet(CL);
		}
		break;
	}
	case CS_CHANGEWEAPON: {
		CS_CHANGEWEAPON_PACKET* p = reinterpret_cast<CS_CHANGEWEAPON_PACKET*>(packet);
		{
			lock_guard<mutex> ll{ CL->_s_lock };
			CL->character_num = (CL->character_num + 1) % 3;
		}
		for (auto& cl : clients[c_id / 4]) {
			if (cl._state == ST_INGAME || cl._state == ST_DEAD)  cl.send_changeweapon_packet(CL);
		}
		break;
	}
	}
}


void worker_thread(HANDLE h_iocp)
{
	while (1) {
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
				CL->Initialize(client_id, g_c_socket);
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
			SESSION* CL = getClient((int)key);	
			int remain_data = num_bytes + CL->_prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p += packet_size;
					remain_data -= packet_size;
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
			OverPool.ReturnMemory(ex_over);
			break;
		case OP_NPC_MOVE:
			int roomNum = static_cast<int>(key) / 100;
			short mon_id = static_cast<int>(key) % 100;
			cout << "RoomNum - " << roomNum << ", m_id - " << mon_id << endl;
			auto iter = find_if(PoolMonsters[roomNum].begin(), PoolMonsters[roomNum].end(), [mon_id](Monster* M) {return M->m_id == mon_id; });
			{
				lock_guard<mutex> mm{ PoolMonsters[roomNum][mon_id]->m_lock };
				monster_update(roomNum, mon_id);
			}
			for (auto& cl : clients[roomNum]) {
				if (cl._state == ST_INGAME || cl._state == ST_DEAD) cl.send_NPCUpdate_packet(*iter);
			}
			if ((*iter)->is_alive() == false) {//0322
				MonsterPool.ReturnMemory((*iter));
				PoolMonsters[roomNum].erase(iter);
				MonsterPool.PrintSize();
			}
			TIMER_EVENT ev{ key / 100, key % 100, high_resolution_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
			timer_queue.push(ev);
			OverPool.ReturnMemory(ex_over);
			break;
		}
	}
}

void update_thread()
{
	//m_PlayerTimer.Start();
	while (1)
	{
		//m_PlayerTimer.Tick(10.f);
		for (int i = 0; i < MAX_ROOM; ++i) {
			for (int j = 0; j < MAX_USER_PER_ROOM; ++j) {
				if (clients[i][j]._state != ST_INGAME) continue;
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
	while (1)
	{
		//m_NPCTimer.Tick(30.0f);
		for (int i = 0; i < MAX_ROOM; ++i) {
			auto iter = PoolMonsters[i].begin();
			while (iter != PoolMonsters[i].end()) {
				{
					lock_guard<mutex> mm{ (*iter)->m_lock }; 
					(*iter)->Update((float)(1.f/30.f));
				}
				for (auto& cl : clients[i]) {
					if (cl._state == ST_INGAME || cl._state == ST_DEAD) cl.send_NPCUpdate_packet((*iter));
				}
				if ((*iter)->is_alive() == false) {//0322
					MonsterPool.ReturnMemory((*iter));
					PoolMonsters[i].erase(iter);
					MonsterPool.PrintSize();
				}
				else
					iter++;
			}
		}
		this_thread::sleep_for(30ms);
	}
}

void do_Timer()
{
	while (1)
	{
		//this_thread::sleep_for(1ms);
		TIMER_EVENT ev;
		auto current_time = high_resolution_clock::now();
		if (timer_queue.try_pop(ev))
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);
				continue;
			}
		switch (ev.event_id) {
		case EV_RANDOM_MOVE:
			OVER_EXP* ov = OverPool.GetMemory();
			ov->_comp_type = OP_NPC_MOVE;
			PostQueuedCompletionStatus(h_iocp, 1, ev.room_id * 100 + ev.obj_id, &ov->_over);
			break;
		}
	}
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
	int ErrorStatus = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorStatus == SOCKET_ERROR) return 0;
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
	//thread* update_player_t = new thread{ update_thread };
	//thread* update_NPC_t = new thread{ update_NPC };
	thread* update_NPC_t = new thread{ do_Timer };
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	//update_player_t->join();
	update_NPC_t->join();
	closesocket(g_s_socket);
	WSACleanup();
}
