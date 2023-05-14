#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
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

	int* m_nObjects = new int(0);
	MapObject** m_ppObjects = LoadGameObjectsFromFile("Models/Scene.bin", m_nObjects);

	for (int i = 0; i < *m_nObjects; i++) {
		if (0 == strcmp(m_ppObjects[i]->m_pstrName, "Dense_Floor_mesh") ||
			0 == strcmp(m_ppObjects[i]->m_pstrName, "Ceiling_concrete_base_mesh") ||
			0 == strcmp(m_ppObjects[i]->m_pstrName, "Bedroom_wall_b_06_mesh") ||
			0 == strcmp(m_ppObjects[i]->m_pstrName, "stone")) continue;

		if (0 == strcmp(m_ppObjects[i]->m_pstrName, "Key_mesh"))
		{ 
			Key_Items[0].emplace_back(m_ppObjects[i]->m_xmOOBB, 1, i);
			continue;
		}
		if (0 == strcmp(m_ppObjects[i]->m_pstrName, "2ndRoomCoin")) {
			Key_Items[1].emplace_back(m_ppObjects[i]->m_xmOOBB, 0.17f, i);
			continue;
		}


		int collide_range_min = ((int)m_ppObjects[i]->m_xmOOBB.Center.z - (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / AREA_SIZE;
		int collide_range_max = ((int)m_ppObjects[i]->m_xmOOBB.Center.z + (int)m_ppObjects[i]->m_xmOOBB.Extents.z) / AREA_SIZE;

		for (int j = collide_range_min; j <= collide_range_max; j++) {
			Objects[j].emplace_back(m_ppObjects[i]);
		}
	}
	delete m_nObjects;
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

	cout << "SERVER READY\n";

	vector <thread> worker_threads;
	vector <thread> timer_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < 1; ++i)
		timer_threads.emplace_back(do_Timer);
	thread* DB_t = new thread{ DB_Thread };
	for (int i = 0; i < num_threads - 1; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	for (auto& th : timer_threads)
		th.join();
	DB_t->join();

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
	SQLWCHAR szUser_ID[NAME_SIZE], szUser_PWD[NAME_SIZE];
	SQLINTEGER dUser_ClearTime, dUser_CurStage;

	setlocale(LC_ALL, "korean");

	SQLLEN cbID = 0, cbPWD = 0, cbClearTime = 0, cbCurStage = 0;

	//// Allocate environment handle  
	//retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	//// Set the ODBC version environment attribute  
	//if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	//	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

	//	// Allocate connection handle  
	//	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	//		retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);


	//		// Set login timeout to 5 seconds  
	//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	//			SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	//			// Connect to data source  
	//			//retcode = SQLConnect(hdbc, (SQLWCHAR*)L"VOODOODOLL_DB", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	//			//SQLWCHAR* ConnStrIn = L"DRIVER={SQL Server};SERVER=127.0.0.1;DATABASE=VOODOODOLL_DB;UID=dbAdmin_master;PWD=2018180005";
	//			retcode = SQLConnect(hdbc, (SQLWCHAR*)L"VOODOODOLL_DB", SQL_NTS, (SQLWCHAR*)L"dbAdmin_master", SQL_NTS, (SQLWCHAR*)L"2018180005", SQL_NTS);
	//			cout << retcode << endl;
	//			// Allocate statement handle  
	//			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	//				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	//				printf("ODBC Connect OK \n");
	//			}
	//			else 									
	//				HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
	//		}
	//	}
	//}

	while (1)
	{
		DB_EVENT ev;
		if (db_queue.try_pop(ev))
		{
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


							wchar_t query[100];
							switch (ev._event) {
							case EV_SIGNUP:
								swprintf(query, L"INSERT INTO user_data (ID, Password, clearTime, curStage) VALUES ('%hs', '%hs', 0, 0)", ev.user_id, ev.user_password);
								printf("%ls\n", query);
								retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query, SQL_NTS);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
									printf("INSERT OK \n");
								}
								else {
									printf("INSERT FAILED \n");
									HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
								}
								break;
							case EV_SIGNIN:
								swprintf(query, L"SELECT ID, Password, clearTime, curStage FROM user_data WHERE ID='%hs' AND Password='%hs'", ev.user_id, ev.user_password);
								printf("%ls\n", query);
								retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query, SQL_NTS);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
									printf("SELECT OK \n");
									OVER_EXP* ov = new OVER_EXP();
									ov->_comp_type = OP_LOGGEDIN;
									PostQueuedCompletionStatus(h_iocp, 1, ev.session_id, &ov->_over);
								}
								else {
									printf("SELECT FAILED \n");
									HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
								}
								break;
							}

							//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT ID, Password, clearTime, curStage FROM user_data", SQL_NTS);
							//if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

							//	printf("Select OK \n");

							//	// Bind columns 1, 2, and 3  
							//	retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szUser_ID, 100, &cbID);
							//	retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szUser_PWD, NAME_SIZE, &cbPWD);
							//	retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &dUser_ClearTime, 100, &cbClearTime);
							//	retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &dUser_CurStage, 100, &cbCurStage);

							//	// Fetch and print each row of data. On an error, display a message and exit.  
							//	for (int i = 0; ; i++) {
							//		retcode = SQLFetch(hstmt);
							//		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
							//			HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
							//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
							//		{
							//			//replace wprintf with printf
							//			//%S with %ls
							//			//warning C4477: 'wprintf' : format string '%S' requires an argument of type 'char *'
							//			//but variadic argument 2 has type 'SQLWCHAR *'
							//			//wprintf(L"%d: %S %S %S\n", i + 1, sCustID, szName, szPhone);  
							//			wprintf(L"%d: %s %s %d %d\n", i + 1, szUser_ID, szUser_PWD, dUser_ClearTime, dUser_CurStage);
							//		}
							//		else
							//			break;
							//	}
							//}

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
		}
		else this_thread::sleep_for(100ms);
	}
}

void process_packet(const int c_id, char* packet)
{
	SESSION& CL = getClient(c_id);
	array<SESSION, MAX_USER_PER_ROOM>& Room = getRoom(c_id);
	if (CL._state.load() == ST_DEAD) {
		return;
	}
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		CL._state.store(ST_INGAME);
		CL.send_login_info_packet();
		for (auto& pl : Room) {
			if (pl._id == c_id || ST_INGAME != pl._state.load()) continue;
			pl.send_add_player_packet(&CL);
			CL.send_add_player_packet(&pl);
		}
		for (auto& monster : getMonsters(c_id))
			CL.send_summon_monster_packet(monster);
		break;
	}
	case CS_SIGNUP: {
		CS_SIGN_PACKET* p = reinterpret_cast<CS_SIGN_PACKET*>(packet);
		DB_EVENT ev;
		ev._event = EV_SIGNUP;
		strncpy_s(ev.user_id, sizeof(ev.user_id), p->id, _TRUNCATE);
		strncpy_s(ev.user_password, sizeof(ev.user_password), p->password, _TRUNCATE);
		ev.session_id = c_id;
		db_queue.push(ev);
		break;
	}
	case CS_SIGNIN: {
		CS_SIGN_PACKET* p = reinterpret_cast<CS_SIGN_PACKET*>(packet);
		DB_EVENT ev;
		ev._event = EV_SIGNIN;
		strncpy_s(ev.user_id, sizeof(ev.user_id), p->id, _TRUNCATE);
		strncpy_s(ev.user_password, sizeof(ev.user_password), p->password, _TRUNCATE);
		ev.session_id = c_id;
		db_queue.push(ev);
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		CL.direction.store(p->direction);
		CL.SetVelocity(p->vel);
		CL.CheckPosition(p->pos);
#ifdef _STRESS_TEST
		CL.recent_recvedTime = p->move_time;
#endif
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_move_packet(&CL);
		}
		break;
	}
	case CS_ROTATE: {
		CS_ROTATE_PACKET* p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		CL.Rotate(p->cxDelta, p->cyDelta, p->czDelta);
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_rotate_packet(&CL);
		}
		break;
	}
	case CS_ATTACK: {
		threadsafe_vector<Monster*>& Monsters = getMonsters(CL._id);
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		XMFLOAT3 _Look = CL.GetLookVector();
		for (auto& cl : Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_attack_packet(&CL);
		}

		switch (CL.character_num)
		{
		case 0:
		{
			p->pos = Vector3::Add(p->pos, Vector3::ScalarProduct(_Look, 10, false));
			shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
			for (auto& monster : Monsters) {
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && Vector3::Length(Vector3::Subtract(p->pos, monster->GetPosition())) < 40)
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
			p->pos = Vector3::Add(p->pos, Vector3::ScalarProduct(_Look, 5, false));
			XMVECTOR Bullet_Origin = XMLoadFloat3(&p->pos);
			XMVECTOR Bullet_Direction = XMLoadFloat3(&_Look);
			vector<Monster*> monstersInRange;

			{
				shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
				for (auto& monster : Monsters) {
					lock_guard<mutex> monster_lock{ monster->m_lock };
					float bullet_monster_distance = Vector3::Length(Vector3::Subtract(monster->BB.Center, p->pos));
					if (monster->HP > 0 && monster->BB.Intersects(Bullet_Origin, Bullet_Direction, bullet_monster_distance))
					{
						monstersInRange.push_back(monster);
					}
				}
			}
			if (!monstersInRange.empty())
			{
				float minDistance = FLT_MAX;
				Monster* closestMonster = nullptr;
				for (auto& monster : monstersInRange)
				{
					lock_guard<mutex> monster_lock{ monster->m_lock };
					float distance = Vector3::Length(Vector3::Subtract(monster->BB.Center, p->pos));
					if (distance < minDistance)
					{
						minDistance = distance;
						closestMonster = monster;
					}
				}
				if (closestMonster)
				{
					lock_guard<mutex> monster_lock{ closestMonster->m_lock };
					int _min = static_cast<int>(min(p->pos.z / AREA_SIZE, closestMonster->BB.Center.z / AREA_SIZE));
					int _max = static_cast<int>(max(p->pos.z / AREA_SIZE, closestMonster->BB.Center.z / AREA_SIZE));
					for (int i = _min; i <= _max; i++)
					{
						for (auto& obj : Objects[i])
						{
							float bullet_obstacle_distance = Vector3::Length(Vector3::Subtract(obj->m_xmOOBB.Center, p->pos));
							if (obj->m_xmOOBB.Intersects(Bullet_Origin, Bullet_Direction, bullet_obstacle_distance) &&
								bullet_obstacle_distance < minDistance) {
								return;
							}
						}
					}
					closestMonster->HP -= 200;
					if (closestMonster->target_id < 0)
						closestMonster->target_id = CL._id;
					if (closestMonster->HP <= 0)
						closestMonster->SetState(NPC_State::Dead);
					return;
				}
			}
			break;
		}
		case 2:
		{
			p->pos = Vector3::Add(p->pos, Vector3::ScalarProduct(_Look, 5, false));
			shared_lock<shared_mutex> vec_lock{ Monsters.v_shared_lock };
			for (auto& monster : Monsters) {
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && Vector3::Length(Vector3::Subtract(p->pos, monster->GetPosition())) < 20)
				{
					monster->HP -= 50;
					if (monster->HP <= 0)
						monster->SetState(NPC_State::Dead);
				}
			}
			break;
		}
		}
		break;
	}
	case CS_INTERACTION: {
		CS_INTERACTION_PACKET* p = reinterpret_cast<CS_INTERACTION_PACKET*>(packet);
		int cur_stage = CL.cur_stage.load();
		for (size_t i = 0; i < Key_Items[cur_stage].size(); i++) {
			if (Key_Items[cur_stage][i].m_xmOOBB.Intersects(CL.m_xmOOBB))
				for (auto& cl : Room) {
					cl.clear_percentage += Key_Items[cur_stage][i].percent;
					if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_interaction_packet(cur_stage, i);
				}
		}

		if (CL.clear_percentage >= 1.f && getMonsters(CL._id).size() <= 0) {
			for (auto& cl : Room) {
				if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_open_door_packet(CL.cur_stage);
				cl.clear_percentage = 0.f;
				if (cl.cur_stage >= 1) cl.clear_percentage = 1.f; // 3번째(코드에선 2번 인덱스) 스테이지부터 퍼즐 미구현이라 임의의 코드로 percent를 100%로 조정함
			}
		}
		break;
	}
	case CS_CHANGEWEAPON: {
		CS_CHANGEWEAPON_PACKET* p = reinterpret_cast<CS_CHANGEWEAPON_PACKET*>(packet);
		CL.character_num = (CL.character_num + 1) % 3;
		//if (CL.character_num == 2)
		//	CL.HP = 100;
		//else CL.HP = 55500;
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
				if (ex_over->_comp_type == OP_SEND)
					delete ex_over;
				//OverPool.ReturnMemory(ex_over);
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND)
				delete ex_over;
			//OverPool.ReturnMemory(ex_over);
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				SESSION& CL = getClient(client_id);
				CL._state.store(ST_ALLOC);
				CL._socket = g_c_socket;
				CL._id = client_id;
				CL.Initialize();	// CL.Initialize를 로그인 이후에 호출하도록 변경해야함
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
		case OP_LOGGEDIN: {
			SESSION& CL = getClient(static_cast<int>(key));
			SC_LOGIN_COMPLETE_PACKET p;
			p.success = true;
			p.type = SC_LOGIN_COMPLETE;
			p.size = sizeof(p);
			CL.do_send(&p);
			//CL.Initialize();
			//CL.send_login_info_packet();
			//CL._state.store(ST_INGAME);
			//for (auto& pl : getRoom(CL._id)) {
			//	if (pl._id == CL._id || ST_INGAME != pl._state.load()) continue;
			//	pl.send_add_player_packet(&CL);
			//	CL.send_add_player_packet(&pl);
			//}
			//CL.send_open_door_packet(0);
			break;
		}
		case OP_RECV: {
			SESSION& CL = getClient(static_cast<int>(key));
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
			delete ex_over;
			//OverPool.ReturnMemory(ex_over);
			break;
		case OP_NPC_MOVE://클라그림자
			int roomNum = static_cast<int>(key) / 100;
			short mon_id = static_cast<int>(key) % 100;
			vector<Monster*>::iterator iter;
			bool found;
			{
				shared_lock<shared_mutex> vec_lock{ PoolMonsters[roomNum].v_shared_lock };
				iter = find_if(PoolMonsters[roomNum].begin(), PoolMonsters[roomNum].end(), [mon_id](Monster* M) {return M->m_id == mon_id; });
				found = (iter != PoolMonsters[roomNum].end());
			}

			if (found) {
				if ((*iter)->alive) {
					{
						lock_guard<mutex> mm{ (*iter)->m_lock };
						(*iter)->Update(duration_cast<milliseconds>(high_resolution_clock::now() - (*iter)->recent_recvedTime).count() / 1000.f);
						(*iter)->recent_recvedTime = high_resolution_clock::now();
					}
					for (auto& cl : clients[roomNum]) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_NPCUpdate_packet(*iter);
					}
					TIMER_EVENT ev{ roomNum, mon_id, (*iter)->recent_recvedTime + 100ms, EV_MOVE };
					timer_queue.push(ev);
				}
				else {
					{
						unique_lock<shared_mutex> vec_lock{ PoolMonsters[roomNum].v_shared_lock };
						iter = find_if(PoolMonsters[roomNum].begin(), PoolMonsters[roomNum].end(), [mon_id](Monster* M) {return M->m_id == mon_id; }); // re-find iter after locking
						if (iter != PoolMonsters[roomNum].end()) {
							PoolMonsters[roomNum].erase(iter);
						}
					}
					MonsterPool.ReturnMemory(*iter);
				}
			}

			delete ex_over;
			//OverPool.ReturnMemory(ex_over);
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
				this_thread::sleep_for(duration_cast<milliseconds>(ev.wakeup_time - current_time));
			}
			switch (ev.event_id) {
			case EV_MOVE:
				OVER_EXP* ov = new OVER_EXP();
				//OVER_EXP* ov = OverPool.GetMemory();
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id * 100 + ev.obj_id, &ov->_over);
				break;
			}
			//EventPool.ReturnMemory(ev);
		}
		else this_thread::sleep_for(1ms);
	}
}

