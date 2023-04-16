#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "main.h"
#include <sqlext.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;
using namespace chrono;

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
HANDLE h_iocp;

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void DB_Thread();
void process_packet(const int c_id, char* packet);
void worker_thread(HANDLE h_iocp);
void do_Timer();


int main()
{
	wcout.imbue(locale("korean"));
	setlocale(LC_ALL, "korean");

	int m_nObjects = 0;
	MapObject** m_ppObjects = LoadGameObjectsFromFile("Models/Scene.bin", &m_nObjects);

	for (int i = 0; i < m_nObjects; i++) {
		if (0 == strncmp(m_ppObjects[i]->m_pstrName, "Dense_Floor_mesh", 16) ||
			0 == strncmp(m_ppObjects[i]->m_pstrName, "Ceiling_concrete_base_mesh", 26)) continue;
		int collide_range_min = ((int)m_ppObjects[i]->m_xmOOBB.Center.z - (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / AREA_SIZE;
		int collide_range_max = ((int)m_ppObjects[i]->m_xmOOBB.Center.z + (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / AREA_SIZE;
		for (int j = collide_range_min; j <= collide_range_max; j++) {
			Objects[j].emplace_back(m_ppObjects[i]);
		}
	}

	//bool col = false;
	//for (float j = 4080; j >= -240; j--) {
	//	for (float i = -360; i < 120; i++) {
	//		for (int k = 0; k < m_nObjects; k++) {
	//			if ((0 == strncmp(m_ppObjects[k]->m_pstrName, "Dense_Floor_mesh", 16) || 0 == strncmp(m_ppObjects[k]->m_pstrName, "Ceiling_concrete_base_mesh", 26)) || 
	//				m_ppObjects[k]->GetPosition().y > -100)
	//				continue;
	//			if (m_ppObjects[k]->m_xmOOBB.Intersects(BoundingBox(XMFLOAT3{ i,-292,j }, XMFLOAT3(15, 12, 8)))) {
	//				col = true;
	//				break;
	//			}
	//		}
	//		if (col) cout << "0";
	//		else cout << "1";
	//		col = false;
	//	}
	//	cout << endl;
	//}

	delete[] m_ppObjects;


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
	vector <thread> timer_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < 2; ++i)
		timer_threads.emplace_back(do_Timer);
	//thread* DB_t = new thread{ DB_Thread };
	for (int i = 0; i < num_threads - 1; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	for (auto& th : timer_threads)
		th.join();
	//DB_t->join();

	closesocket(g_s_socket);
	WSACleanup();
}




void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) {
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated.. 
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void DB_Thread()
{
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szUser_Name[NAME_SIZE];
	SQLINTEGER dUser_id, dUser_PlayTime;

	setlocale(LC_ALL, "korean");

	SQLLEN cbName = 0, cbID = 0, cbLevel = 0;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"VOODOODOLL_DB", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					printf("ODBC Connect OK \n");

					//SELECT ������ ���°� �׸��, FROM ������ ���°� ���̺� �̸�, ORDER BY ������ ���°� sorting �� ��. ������ user_name���� ����.
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT ID, Name, PlayTime FROM user_data ORDER BY 3", SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

						printf("Select OK \n");

						// Bind columns 1, 2, and 3  
						retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &dUser_id, 100, &cbID);
						retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szUser_Name, NAME_SIZE, &cbName);
						retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &dUser_PlayTime, 100, &cbLevel);

						// Fetch and print each row of data. On an error, display a message and exit.  
						for (int i = 0; ; i++) {
							retcode = SQLFetch(hstmt);
							if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
								HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
							{
								//replace wprintf with printf
								//%S with %ls
								//warning C4477: 'wprintf' : format string '%S' requires an argument of type 'char *'
								//but variadic argument 2 has type 'SQLWCHAR *'
								//wprintf(L"%d: %S %S %S\n", i + 1, sCustID, szName, szPhone);  
								wprintf(L"%d: %d %s %d\n", i + 1, dUser_id, szUser_Name, dUser_PlayTime);
							}
							else
								break;
						}
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}

	system("pause");
}

void process_packet(const int c_id, char* packet)
{
	SESSION& CL = getClient(c_id);
	array<SESSION, MAX_USER_PER_ROOM>& Room = getRoom(c_id);
	if (CL._state.load() == ST_DEAD) {
		cout << "[" << CL._id << "] player sent packet in dead_state\n";
		CL.error_stack++;
		return;
	}
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		CL.send_login_info_packet();
		CL._state.store(ST_INGAME);
		for (auto& pl : Room) {
			if (pl._id == c_id || ST_INGAME != pl._state.load()) continue;
			pl.send_add_player_packet(&CL);
			CL.send_add_player_packet(&pl);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		CL.direction.store(p->direction);
		CL.Rotate(p->cxDelta, p->cyDelta, p->czDelta);
		CL.SetVelocity(p->vel);
		CL.Update();
		CL.CheckPosition(p->pos);
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_move_packet(&CL);
		}
		break;
	}
	case CS_ATTACK: {
		threadsafe_vector<Monster*>& Monsters = getMonsters(CL._id);
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		switch (CL.character_num)
		{
		case 0:
		{
			shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
			for (auto& monster : Monsters) {
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && BoundingBox(p->pos, { 15,1,15 }).Intersects(monster->BB))
				{
					monster->HP -= 100;
					if (monster->HP <= 0)
						monster->SetState(NPC_State::Dead);
				}
			}
			break;
		}
		case 1:
		{
			CL.BulletLook = CL.GetLookVector();
			CL.BulletPos = Vector3::Add(CL.GetPosition(), XMFLOAT3(0, 10, 0));
			break;
		}
		case 2:
		{
			shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
			for (auto& monster : Monsters) {
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && BoundingBox(p->pos, { 5,1,5 }).Intersects(monster->BB))
				{
					monster->HP -= 50;
					if (monster->HP <= 0)
						monster->SetState(NPC_State::Dead);
				}
			}
			break;
		}
		}
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_attack_packet(&CL);
		}
		break;
	}
	case CS_COLLECT: {
		CS_COLLECT_PACKET* p = reinterpret_cast<CS_COLLECT_PACKET*>(packet);
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_collect_packet(&CL);
		}
		break;
	}
	case CS_CHANGEWEAPON: {
		CS_CHANGEWEAPON_PACKET* p = reinterpret_cast<CS_CHANGEWEAPON_PACKET*>(packet);
		CL.character_num = (CL.character_num + 1) % 3;

		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_changeweapon_packet(&CL);
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
			if (ex_over->_comp_type == OP_ACCEPT) { cout << "Accept Error"; exit(-1); }
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) OverPool.ReturnMemory(ex_over);
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) OverPool.ReturnMemory(ex_over);
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			SESSION& CL = getClient(client_id);
			if (client_id != -1) {
				CL._state.store(ST_ALLOC);
				CL.Initialize(client_id, g_c_socket);
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				CL.do_recv();
			}
			else {
				cout << "Max user exceeded.\n";
				closesocket(g_c_socket);
			}
			g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			SESSION& CL = getClient((int)key);
			int remain_data = num_bytes + CL._prev_remain;
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
			CL._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			CL.do_recv();
			break;
		}
		case OP_SEND:
			OverPool.ReturnMemory(ex_over);
			break;
		case OP_NPC_MOVE://04166
			int roomNum = static_cast<int>(key) / 100;
			short mon_id = static_cast<int>(key) % 100;
			{
				unique_lock<shared_mutex> vec_lock{ PoolMonsters[roomNum].v_shared_lock };
				auto iter = find_if(PoolMonsters[roomNum].begin(), PoolMonsters[roomNum].end(), [mon_id](Monster* M) {return M->m_id == mon_id; });

				if (iter != PoolMonsters[roomNum].end()) {
					if ((*iter)->is_alive()) {
						{
							{
								lock_guard<mutex> mm{ (*iter)->m_lock };
								(*iter)->Update(duration_cast<milliseconds>(high_resolution_clock::now() - (*iter)->recent_recvedTime).count() / 1000.f);
								(*iter)->recent_recvedTime = high_resolution_clock::now();
							}
							for (auto& cl : clients[roomNum]) {
								if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_NPCUpdate_packet(*iter);
							}
							TIMER_EVENT ev{ roomNum, mon_id, (*iter)->recent_recvedTime + 100ms, EV_RANDOM_MOVE };
							timer_queue.push(ev);
						}
					}
					else {
						MonsterPool.ReturnMemory(*iter);
						PoolMonsters[roomNum].erase(iter);
					}
				}
			}
			OverPool.ReturnMemory(ex_over);
			break;
		}
	}
}



void do_Timer()
{
	while (1)
	{
		TIMER_EVENT ev;
		auto current_time = high_resolution_clock::now();
		if (timer_queue.try_pop(ev)) {
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
}

