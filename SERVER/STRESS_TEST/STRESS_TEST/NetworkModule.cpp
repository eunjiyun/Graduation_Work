#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <array>
#include <memory>

using namespace std;
using namespace chrono;

extern HWND		hWnd;

const static int MAX_TEST = 1000;
const static int MAX_CLIENTS = MAX_TEST * 2;
const static int INVALID_ID = -1;
const static int MAX_PACKET_SIZE = 512;
const static int MAX_BUFF_SIZE = 512;

#pragma comment (lib, "ws2_32.lib")

#include "protocol.h"

HANDLE g_hiocp;

enum OPTYPE { OP_SEND, OP_RECV, OP_DO_MOVE };

high_resolution_clock::time_point last_connect_time;

struct OverlappedEx {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	char IOCP_buf[MAX_BUFF_SIZE];
	OPTYPE event_type;
};

struct CLIENT {
	int id;
	XMFLOAT3 pos;
	atomic_bool connected;

	SOCKET client_socket;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_packet_data;
	int curr_packet_size;
	high_resolution_clock::time_point last_move_time;
	high_resolution_clock::time_point last_recved_time;
};

struct MONSTER {
	XMFLOAT3 pos;
	atomic_bool connected;
	high_resolution_clock::time_point last_recved_time;
};

array<int, MAX_CLIENTS> client_map;
array<CLIENT, MAX_CLIENTS> g_clients;
array<MONSTER, MAX_CLIENTS> g_monsters;
atomic_int num_connections;
atomic_int client_to_close;
atomic_int active_clients;
atomic_int active_monsters;

int			player_delay;
int			monster_delay;				

vector <thread*> worker_threads;
thread test_thread;

float point_cloud[MAX_TEST * 2];
float point_cloud_2[MAX_TEST * 2];

// 나중에 NPC까지 추가 확장 용
struct ALIEN {
	int id;
	int x, y;
	int visible_count;
};

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	// while (true);
}

void DisconnectClient(int ci)
{
	bool status = true;
	if (true == atomic_compare_exchange_strong(&g_clients[ci].connected, &status, false)) {
		closesocket(g_clients[ci].client_socket);
		active_clients--;
	}
	// cout << "Client [" << ci << "] Disconnected!\n";
}

void SendPacket(int cl, void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];
	OverlappedEx* over = new OverlappedEx;
	over->event_type = OP_SEND;
	memcpy(over->IOCP_buf, packet, psize);
	ZeroMemory(&over->over, sizeof(over->over));
	over->wsabuf.buf = reinterpret_cast<CHAR*>(over->IOCP_buf);
	over->wsabuf.len = psize;
	int ret = WSASend(g_clients[cl].client_socket, &over->wsabuf, 1, NULL, 0,
		&over->over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket:", err_no);
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void ProcessPacket(int ci, unsigned char packet[])
{
	switch (packet[1]) {
	case SC_MOVE_PLAYER: {
		SC_MOVE_PLAYER_PACKET* move_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(packet);
		if (move_packet->id < MAX_CLIENTS) {
			int my_id = client_map[move_packet->id];
			if (move_packet->HP <= 0)
				g_clients[my_id].connected = false;
			if (-1 != my_id) {
				g_clients[my_id].pos = move_packet->Pos;
				//cout << g_clients[my_id].pos.x << ", " << g_clients[my_id].pos.y << ", " << g_clients[my_id].pos.z << endl;
			}
			if (ci == my_id) {
				auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - move_packet->move_time;
				if (player_delay < d_ms) player_delay++;
				else if (player_delay > d_ms) player_delay--;
				if (move_packet->HP <= 0) { DisconnectClient(my_id); }
			}
		}
	}
					   break;
	case SC_ADD_PLAYER: break;
	case SC_REMOVE_PLAYER: break;
	case SC_LOGIN_INFO:
	{
		g_clients[ci].connected = true;
		active_clients++;
		SC_LOGIN_INFO_PACKET* login_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		int my_id = ci;
		client_map[login_packet->id] = my_id;
		g_clients[my_id].id = login_packet->id;
		g_clients[my_id].pos = login_packet->pos;
		//cs_packet_teleport t_packet;
		//t_packet.size = sizeof(t_packet);
		//t_packet.type = CS_TELEPORT;
		//SendPacket(my_id, &t_packet);
	}
	break;
	case SC_SUMMON_MONSTER:
	{
		if (ci % MAX_USER_PER_ROOM == 1) {
			SC_SUMMON_MONSTER_PACKET* p = reinterpret_cast<SC_SUMMON_MONSTER_PACKET*>(packet);
			//cout << p->room_num << "몬스터 소환\n";
			g_monsters[p->room_num * 10 + p->id].connected = true;
			active_monsters++;
			g_monsters[p->room_num * 10 + p->id].pos = p->Pos;
			g_monsters[p->room_num * 10 + p->id].last_recved_time = high_resolution_clock::now();
		}
	}
	break;
	case SC_MOVE_MONSTER:
	{
		if (ci % MAX_USER_PER_ROOM == 1) {
			SC_MOVE_MONSTER_PACKET* p = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(packet);
			if (p->is_alive == false) {
				g_monsters[p->room_num * 10 + p->id].connected = false;
				active_monsters--;
				break;
			}
			g_monsters[p->room_num * 10 + p->id].pos = p->Pos;

			auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now() - g_monsters[p->room_num * 10 + p->id].last_recved_time).count();
			if (monster_delay < d_ms) monster_delay++;
			else if (monster_delay > d_ms) monster_delay--;
			g_monsters[p->room_num * 10 + p->id].last_recved_time = high_resolution_clock::now();
		}

		break;
	}
	case CS_ATTACK: {
		break;
	}
	case CS_CHANGEWEAPON: {
		break;
	}
	case SC_OPEN_DOOR: {
		break;
	}
	case SC_ROTATE_PLAYER: {
		break;
	}
	default: {
		break;
	}
	}
}

void Worker_Thread()
{
	while (true) {
		DWORD io_size;
		ULONG_PTR ci;
		//OverlappedEx* over;
		WSAOVERLAPPED* ex_over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_hiocp, &io_size, &ci, &ex_over, INFINITE);
		OverlappedEx* over = reinterpret_cast<OverlappedEx*>(ex_over);
		int client_id = static_cast<int>(ci);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(client_id);
			else {
				// error_display("GQCS : ", WSAGetLastError());
				DisconnectClient(client_id);
			}
			if (OP_SEND == over->event_type) delete over;
		}
		if (0 == io_size) {
			DisconnectClient(client_id);
			continue;
		}
		if (OP_RECV == over->event_type) {
			char* buf = g_clients[ci].recv_over.IOCP_buf;
			unsigned psize = g_clients[ci].curr_packet_size;
			unsigned pr_size = g_clients[ci].prev_packet_data;
			while (io_size > 0) {
				if (0 == psize) psize = buf[0];
				if (io_size + pr_size >= psize) {
					// 지금 패킷 완성 가능
					unsigned char packet[MAX_PACKET_SIZE];
					memcpy(packet, g_clients[ci].packet_buf, pr_size);
					memcpy(packet + pr_size, buf, psize - pr_size);
					ProcessPacket(static_cast<int>(ci), packet);
					io_size -= psize - pr_size;
					buf += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(g_clients[ci].packet_buf + pr_size, buf, io_size);
					pr_size += io_size;
					io_size = 0;
				}
			}
			g_clients[ci].curr_packet_size = psize;
			g_clients[ci].prev_packet_data = pr_size;
			DWORD recv_flag = 0;
			int ret = WSARecv(g_clients[ci].client_socket,
				&g_clients[ci].recv_over.wsabuf, 1,
				NULL, &recv_flag, &g_clients[ci].recv_over.over, NULL);
			if (SOCKET_ERROR == ret) {
				int err_no = WSAGetLastError();
				if (err_no != WSA_IO_PENDING)
				{
					//error_display("RECV ERROR", err_no);
					DisconnectClient(client_id);
				}
			}
		}
		else if (OP_SEND == over->event_type) {
			if (io_size != over->wsabuf.len) {
				// std::cout << "Send Incomplete Error!\n";
				DisconnectClient(client_id);
			}
			delete over;
		}
		else if (OP_DO_MOVE == over->event_type) {
			// Not Implemented Yet
			delete over;
		}
		else {
			std::cout << "Unknown GQCS event! - " << io_size << endl;
			while (true);
		}
	}
}

constexpr int DELAY_LIMIT = 100;
constexpr int DELAY_LIMIT2 = 150;
constexpr int MONSTER_DELAY_LIMIT = 200;
constexpr int MONSTER_DELAY_LIMIT2 = 250;
constexpr int ACCEPT_DELY = 50;

void Adjust_Number_Of_Client()
{
	static int delay_multiplier = 1;
	static int max_limit = MAXINT;
	static bool increasing = true;

	if (active_clients >= MAX_TEST) return;
	if (num_connections >= MAX_CLIENTS) return;

	auto duration = high_resolution_clock::now() - last_connect_time;
	if (ACCEPT_DELY * delay_multiplier > duration_cast<milliseconds>(duration).count()) return;

	int t_delay = player_delay;
	int t_delay_2 = monster_delay;
	if (DELAY_LIMIT2 < t_delay || MONSTER_DELAY_LIMIT2 < t_delay_2) {
		if (true == increasing) {
			max_limit = active_clients;
			increasing = false;
		}
		if (100 > active_clients) return;
		if (ACCEPT_DELY * 10 > duration_cast<milliseconds>(duration).count()) return;
		last_connect_time = high_resolution_clock::now();
		DisconnectClient(client_to_close);
		client_to_close++;
		return;
	}
	else
		if (DELAY_LIMIT < t_delay || MONSTER_DELAY_LIMIT < t_delay_2) {
			delay_multiplier = 10;
			return;
		}
	if (max_limit - (max_limit / 20) < active_clients) return;

	increasing = true;
	last_connect_time = high_resolution_clock::now();
	g_clients[num_connections].client_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT_NUM);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


	int Result = WSAConnect(g_clients[num_connections].client_socket, (sockaddr*)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (0 != Result) {
		error_display("WSAConnect : ", GetLastError());
	}

	g_clients[num_connections].curr_packet_size = 0;
	g_clients[num_connections].prev_packet_data = 0;
	ZeroMemory(&g_clients[num_connections].recv_over, sizeof(g_clients[num_connections].recv_over));
	g_clients[num_connections].recv_over.event_type = OP_RECV;
	g_clients[num_connections].recv_over.wsabuf.buf =
		reinterpret_cast<CHAR*>(g_clients[num_connections].recv_over.IOCP_buf);
	g_clients[num_connections].recv_over.wsabuf.len = sizeof(g_clients[num_connections].recv_over.IOCP_buf);

	DWORD recv_flag = 0;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_clients[num_connections].client_socket), g_hiocp, num_connections, 0);

	CS_LOGIN_PACKET l_packet;

	int temp = num_connections;
	//sprintf_s(l_packet.name, "%d", temp);
	l_packet.size = sizeof(CS_LOGIN_PACKET);
	l_packet.type = CS_LOGIN;
	SendPacket(num_connections, &l_packet);


	int ret = WSARecv(g_clients[num_connections].client_socket, &g_clients[num_connections].recv_over.wsabuf, 1,
		NULL, &recv_flag, &g_clients[num_connections].recv_over.over, NULL);
	if (SOCKET_ERROR == ret) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			error_display("RECV ERROR", err_no);
			goto fail_to_connect;
		}
	}
	num_connections++;
fail_to_connect:

	return;
}

void Test_Thread()
{
	while (true) {
		//Sleep(max(20, global_delay));
		Adjust_Number_Of_Client();

		for (int i = 0; i < num_connections; ++i) {
			if (false == g_clients[i].connected) continue;
			if (g_clients[i].last_move_time + 100ms > high_resolution_clock::now()) continue;
			g_clients[i].last_move_time = high_resolution_clock::now();
			CS_MOVE_PACKET my_packet;
			my_packet.size = sizeof(my_packet);
			my_packet.type = CS_MOVE;
			my_packet.direction = rand() % 8;
			short packet_type = rand() % 10;
			if (packet_type <= 6) {
				if (my_packet.direction & 1) g_clients[i].pos = Vector3::Add(g_clients[i].pos, XMFLOAT3(0, 0, 1));
				//if (my_packet.direction & 2) g_clients[i].pos = Vector3::Add(g_clients[i].pos, XMFLOAT3(0, 0, -3));
				if (my_packet.direction & 2) g_clients[i].pos = Vector3::Add(g_clients[i].pos, XMFLOAT3(-1, 0, 0));
				if (my_packet.direction & 4) g_clients[i].pos = Vector3::Add(g_clients[i].pos, XMFLOAT3(1, 0, 0));
				my_packet.pos = g_clients[i].pos;
				my_packet.id = i;
				my_packet.move_time = static_cast<unsigned>(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
				SendPacket(i, &my_packet);
			}
			else if (packet_type <= 8) {
				CS_ATTACK_PACKET my_packet_2;
				my_packet_2.size = sizeof(CS_ATTACK_PACKET);
				my_packet_2.type = CS_ATTACK;
				my_packet_2.id = i;
				my_packet_2.pos = g_clients[i].pos;
				my_packet.move_time = static_cast<unsigned>(duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
				SendPacket(i, &my_packet_2);
			}
			else {
				CS_CHANGEWEAPON_PACKET my_packet_3;
				my_packet_3.size = sizeof(CS_CHANGEWEAPON_PACKET);
				my_packet_3.type = CS_CHANGEWEAPON;
				my_packet_3.id = i;
				SendPacket(i, &my_packet_3);
				break;
			}
		}
	}
}

void InitializeNetwork()
{
	for (auto& cl : g_clients) {
		cl.connected = false;
		cl.id = INVALID_ID;
	}

	for (auto& cl : client_map) cl = -1;
	num_connections = 0;
	last_connect_time = high_resolution_clock::now();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	for (int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	test_thread = thread{ Test_Thread };
}

void ShutdownNetwork()
{
	test_thread.join();
	for (auto pth : worker_threads) {
		pth->join();
		delete pth;
	}
}

void Do_Network()
{
	return;
}

void GetPointCloud(int* size, int* size_2, float** points, float** points_2)
{
	int index = 0;
	int index_2 = 0;
	for (int i = 0; i < num_connections; ++i)
		if (true == g_clients[i].connected) {
			point_cloud[index * 2] = g_clients[i].pos.x;
			point_cloud[index * 2 + 1] = g_clients[i].pos.z;
			index++;
		}

	
	for (int i = 0; i < MAX_CLIENTS; ++i)
		if (g_monsters[i].connected == true)
		{
			point_cloud_2[index_2 * 2] = g_monsters[i].pos.x;
			point_cloud_2[index_2 * 2 + 1] = g_monsters[i].pos.z;
			index_2++;
		}

	*size = index;
	*points = point_cloud;
	*size_2 = index_2;
	*points_2 = point_cloud_2;
}

