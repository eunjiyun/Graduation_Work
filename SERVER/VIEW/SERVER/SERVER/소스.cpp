#include <iostream>
#include <array>	// ���ǵ��� �迭�� �����ϱ� ���� array ���
#include <WS2tcpip.h>
#include <MSWSock.h>	// acceptEX ����� ���� ����� ���
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
//#include <concurrent_unordered_set.h>	// Set �����̳ʸ� ����ŷ ������ ����� ���� �ʿ������ �ϴ� ������� �������� �����Ƿ� ��������
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")	// acceptEX ����� ���� ����� ���̺귯��
using namespace std;
constexpr int MAX_USER = 10;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
class OVER_EXP {		// �ΰ������ �پ� �� Ȯ��� overlapped ����ü
public:
	WSAOVERLAPPED _over;		// overlapped ����ü
	WSABUF _wsabuf;				// WSA����
	char _send_buf[BUF_SIZE];	// ���ڿ� ����
	COMP_TYPE _comp_type;	// accept�ΰ� recv�ΰ� send�ΰ�
	OVER_EXP()					// ������ ���� �����ϸ� recv������ ������
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)		// �����͸� ���ڷ� �ְ� �����ϸ� send������ ������
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
	OVER_EXP _recv_over;

public:
	mutex s_lock;
	S_STATE _state;
	//bool in_use; 
	int _id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;

	unordered_set<int> view_list; // �þ� ó���� ���� ���� �����̳�. ���̵� �ְ� ���� �۾��� ���� �����Ƿ� set�� ���� �����ϸ�, ������ �ʿ䰡 �����Ƿ� unordered�� ����Ѵ�.
	mutex _vl;


public:
	SESSION()
	{
		_socket = 0;
		_id = -1;
		x = y = 0;
		_name[0] = 0;		// �̸��� Ŭ���̾�Ʈ�� �������� ������ �� ���� ����
		//in_use = false;		// ���� ������ ����������� �����ִ� bool ����(������̸� true, ������ false)
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}		// ���� ������ ������ �迭�� ���α׷� ����ñ��� �Ҹ��ڰ� ȣ����� �ʴ´�.

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;	// prev_remain�� �ִ¸�ŭ ������ ũ�Ⱑ �پ���� �߸� �����͸� �̾���� �� �ִ�.
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };	// ��Ŷ�� ���ڷ� �� send�� overalpped ����ü ����
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()	// Ŭ���̾�Ʈ�� �α��� ���� ��Ŷ ����
	{
		SC_LOGIN_INFO_PACKET p;		// �α��� ������ ��� ��Ŷ. �� ��Ŷ�� ���� ���������� ������ Ŭ���̾�Ʈ�� �����Ѵ�.
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_remove_player_packet(int c_id);
};

array<SESSION, MAX_USER> clients;	// clients �����̳ʴ� ���� �÷��� ���� ���� ��� �Լ����� ���ÿ� ���� �����尡 �����ϹǷ� data race�� �ſ� ����
// ��Ƽ������ ���α׷��ֿ��� data race�� ���� ������ �����̳ʴ� array�̴�.(���ͳ� ����Ʈ���� �� ������)
// �� �׷����ѵ� �����̳ʰ� ������ ���� �����̳� ���� SESSION�� ���Ҹ� ������ �� Data Race�� ���� �� ���� -> ���ؽ��� ����.
SOCKET g_c_socket, g_s_socket;	// g_c_socket, g_s_socket, g_a_over ������ ���� �����尡 ���ÿ� �������� �����Ƿ� data race�� �����ϴ�.
OVER_EXP g_a_over;


bool can_see(int from, int to)
{
	if (abs(clients[from].x - clients[to].x) > VIEW_RANGE)	// �þ� ������ ��Ÿ���� ���� VIEW_RANGE�� �������� ��� 4�� ��������
		return false;
	return abs(clients[from].y - clients[to].y <= VIEW_RANGE);

}


void SESSION::send_move_packet(int c_id)	// Ŭ���̾�Ʈ�� ������ ���� ��Ŷ ����
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	do_send(&p);
}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(SC_MOVE_PLAYER_PACKET);
	add_packet.type = SC_MOVE_PLAYER;
	add_packet.x = clients[c_id].x;
	add_packet.y = clients[c_id].y;
	_vl.lock();
	view_list.insert(c_id);
	_vl.unlock();
	do_send(&add_packet);
}

void SESSION::send_remove_player_packet(int c_id)
{
	_vl.lock(); // ��
	if (view_list.count(c_id))	// ���� ��Ҹ� erase�Ϸ��� �ϸ� ���α׷��� �׾�����ϱ� �˻��ϰ�
		view_list.erase(c_id);
	else {
		_vl.unlock();
		return;
	}
	_vl.unlock();
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	do_send(&p);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) // �����̳ʿ��� �� �ڸ� Ž��
	{
		lock_guard<mutex> ll{ clients[i].s_lock };	// �� ���带 ����ϸ� return ������ ���߿� �Լ��� ����ŵ� ���� �����ϰ� ������ 
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;		// ������ ���� �Ұ���
}

void process_packet(int c_id, char* packet)
{
	// ��� ��Ŷ�� ó���� ����ִ� �ſ� �߿��� �Լ�
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);	// ��Ŷ�� �α��� ��Ŷ���� ĳ����
		strcpy_s(clients[c_id]._name, p->name); // �̸� ����
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id].s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {	// ������ �������� Ŭ���̾�Ʈ�鿡�� �� Ŭ���̾�Ʈ�� �����ߴٴ� ������ ������ �Ѵ�. 

			{
				lock_guard<mutex> ll{ pl.s_lock };
				if (pl._state != ST_INGAME) continue;	// ���� ���� Ŭ��� ����
			}
			if (pl._id == c_id) continue;
			if (false == can_see(c_id, pl._id))	// �� Ŭ���̾�Ʈ�� �� Ŭ���̾�Ʈ�� �� �� ������(�þ� �ȿ� ������)
				continue;

			pl.send_add_player_packet(c_id);

		}
		break;
	}
	case CS_MOVE: {	// �Է��� �޾� Ŭ���̾�Ʈ�� ��ġ�� �ٲٴ� �Լ�
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;

		unordered_set<int> near_list;
		clients[c_id]._vl.lock();
		unordered_set<int> old_vlist = clients[c_id].view_list;	// ����ȭ�� ���� �丮��Ʈ
		clients[c_id]._vl.unlock();


		for (auto& pl : clients)
		{
			if (pl._state != ST_INGAME) continue;
			if (pl._id == c_id) continue;
			if (can_see(c_id, pl._id))
				near_list.insert(pl._id);
		}

		clients[c_id].send_move_packet(c_id);
		for (auto& pl : near_list) {
			auto& cpl = clients[pl];
			cpl._vl.lock();	// view_list�� �����ϴ� ���� data race�� ������ �����Ƿ� ���� �����
			if (cpl.view_list.count(c_id))	// ������ view_list�� �� id�� �ִ���(�� ���� �ִ���) üũ 
			{
				cpl._vl.unlock();
				cpl.send_move_packet(c_id);
			}
			else {
				cpl._vl.unlock();
				cpl.send_add_player_packet(c_id);	// ������ �� �丮��Ʈ�� �߰�����
			}

			if (old_vlist.count(pl) == 0)	// ���� �� �丮��Ʈ�� ��밡 ������ ��븦 �߰����ֱ� 
				clients[c_id].send_add_player_packet(pl);	// ���� ����� �ι� ���� ������ ���� �ѽö� ���� ���� ���ؼ���(����)
		}

		for (auto& pl : old_vlist) {
			if (0 == near_list.count(pl)) {
				clients[c_id].send_remove_player_packet(pl);
				clients[pl].send_remove_player_packet(c_id);
			}

		}
		break;
	}
	}
}

void disconnect(int c_id)	// Ŭ���̾�Ʈ���� ������ ���� �Լ�
{
	clients[c_id]._vl.lock();
	unordered_set<int> vl = clients[c_id].view_list;
	clients[c_id]._vl.unlock();
	for (auto& p_id : vl) {	// ��� Ŭ���̾�Ʈ���� ������ ���·� �ǵ���
		auto& pl = clients[p_id];
		{
			lock_guard<mutex> ll{ pl.s_lock };
			if (pl._state != ST_INGAME) continue;	// ���� ���� Ŭ��� ����
		}
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
	closesocket(clients[c_id]._socket);


	lock_guard<mutex> ll{ clients[c_id].s_lock };

	clients[c_id]._state = ST_FREE;
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {		// ���� �Լ� ���� ����
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE); // IOCP���� IO �ϷḦ ����ϴ� �Լ�(INFINITE)
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";	// acceptŸ�� ����ü���� ������ �߻��� ��� �������� ���
			else {
				// GQCS�� ������ �߻��� ��� disconnect�� ������ �����ش�.
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {	// ���� �ڸ��� ������ ���ο� Ŭ���̾�Ʈ ����
				{
					lock_guard<mutex> ll{ clients[client_id].s_lock };
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);

				clients[client_id].do_recv();	// accept�� �Ϸ�Ǹ� ��� recv�� �ɾ��
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else { // Ŭ���̾�Ʈ ���̵� �� ���� ��� ����á�ٴ� ���� ���
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {	// recv�� ��Ŷ�� �������Ͽ� �ùٸ��� ������ �� ��Ŷ�� �´� ���μ����� �����Ѵ�.
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
			if (remain_data > 0) {	// �����ִ� �����Ͱ� ���� ���
				memcpy(ex_over->_send_buf, p, remain_data);	// ���ۿ� �����صд�.
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

int main()
{
	HANDLE h_iocp;

	WSADATA WSAData;
	int ErrorStatus = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorStatus != 0)
	{
		cout << "WSAStartup ����\n";
		return 0;
	}

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
	int client_id = 0;

	// Accept �ϱ� ���� ���� ������ IOCP�� ������־�� �Ѵ�.
	// CreateIoCompletionPort���� �� ���� ����� �ִ�.
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);						// 1. INVALID_HANDLE_VALUE�� ���ڷ� �־� IOCP �ڵ��� ���� �� ����
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);		// 2. ����̽� �ڵ��� �����Ͽ� IOCP�� ����̽��� ����. ������ �ڵ� ����
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;	// accept Ÿ�Ը� ������. WSA���۳� ���ڿ� ���� ���� �� �� �ʿ䰡 ����(accept�� �ϹǷ�)

	// AcceptEx - �񵿱� ������� �۵��ϴ� Accept �Լ�
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);


	// ��Ƽ�����带 ���� �ڵ�
	// GQCS�� �����忡�� ����Ǳ� ������ �����帶�� �����Ǵ� �����͸� �Ű��� �Ѵ�.
	// �����尡 �����ϴ� �����͸� �����ϱ� ���� ��κ��� �����Ͱ� ��������ȭ�Ǿ���.
	// ���� ������ �����Ǿ� �߻��ϴ� data race�� �����ؾ� �Ѵ�.


	vector<thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();	// ���� CPU�� ���� �ھ� ��
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();




	closesocket(g_s_socket);
	WSACleanup();
}