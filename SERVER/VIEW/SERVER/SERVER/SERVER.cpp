#include <iostream>
#include <WS2tcpip.h>
#include <unordered_map>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 200;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

class SESSION;
unordered_map<long long, SESSION> clients;
class SESSION {
	SOCKET client;
	WSAOVERLAPPED c_over;
	WSABUF c_wsabuf[1];
	long long m_id;
public:
	CHAR c_mess[BUF_SIZE];
	SESSION() { exit(-1); }
	SESSION(int id, SOCKET so) : m_id(id), client(so)
	{
		c_wsabuf[0].buf = c_mess;
	}
	~SESSION()
	{
		closesocket(client);
	}
	void do_recv()
	{
		c_wsabuf[0].len = BUF_SIZE;
		DWORD recv_flag = 0;
		memset(&c_over, 0, sizeof(c_over));
		c_over.hEvent = reinterpret_cast<HANDLE>(m_id);
		WSARecv(client, c_wsabuf, 1, 0, &recv_flag, &c_over, recv_callback);
	}
	void do_send(int num_bytes)
	{
		c_wsabuf[0].len = num_bytes;
		memset(&c_over, 0, sizeof(c_over));
		c_over.hEvent = reinterpret_cast<HANDLE>(m_id);
		WSASend(client, c_wsabuf, 1, 0, 0, &c_over, send_callback);
	}
};

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	long long client_id = reinterpret_cast<long long>(over->hEvent);
	//cout << "Client[" << client_id << "] sent: " << clients[client_id].c_mess << endl;
	clients[client_id].do_send(num_bytes);
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	long long client_id = reinterpret_cast<long long>(over->hEvent);
	clients[client_id].do_recv();
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	for (int i = 1; ; ++i) {
		SOCKET client = WSAAccept(server,
			reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, NULL, NULL);
		printf("%d번째 플레이어 접속\n", i);
		clients.try_emplace(i, i, client);
		clients[i].do_recv();
	}
	closesocket(server);
	WSACleanup();
}
