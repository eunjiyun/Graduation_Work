#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "MyThread.h"
#include <sqlext.h>
#include <sql.h>
#include <sqltypes.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;
using namespace chrono;

int main()
{
	// 콘솔 한글 출력 지원
	wcout.imbue(locale("korean"));
	setlocale(LC_ALL, "korean");

	// 맵, 몬스터정보 등 게임 환경 초기화
	InitializeMap();
	InitializeMonsterInfo();
	InitializeGrid();
	InitializeMonsters();

	// 서버 소켓 생성 -> 바인딩
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

	// IOCP 초기화
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);

	// 클라이언트 연결 승인
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);


	cout << "SERVER READY\n";

	// CPU 코어 개수에 따른 스레드 생성 및 역할 분배
	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	thread Event_Thread{ Timer_Thread };
	thread Database_thread{ DB_Thread };
	for (int i = 0; i < num_threads - 1; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);


	for (auto& th : worker_threads)
		th.join(); 
	Event_Thread.join();
	Database_thread.join();

	FinalizeMonsters();

	closesocket(g_s_socket);
	WSACleanup();
}








