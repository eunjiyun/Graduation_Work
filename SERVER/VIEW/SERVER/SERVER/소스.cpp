#include <iostream>
#include <array>	// 세션들을 배열로 저장하기 위해 array 사용
#include <WS2tcpip.h>
#include <MSWSock.h>	// acceptEX 사용을 위해 사용한 헤더
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
//#include <concurrent_unordered_set.h>	// Set 컨테이너를 논블로킹 구조로 만들어 락이 필요없도록 하는 헤더지만 문제점이 많으므로 쓰지않음
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")	// acceptEX 사용을 위해 사용한 라이브러리
using namespace std;
constexpr int MAX_USER = 10;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
class OVER_EXP {		// 부가기능이 붙어 더 확장된 overlapped 구조체
public:
	WSAOVERLAPPED _over;		// overlapped 구조체
	WSABUF _wsabuf;				// WSA버퍼
	char _send_buf[BUF_SIZE];	// 문자열 버퍼
	COMP_TYPE _comp_type;	// accept인가 recv인가 send인가
	OVER_EXP()					// 데이터 없이 생성하면 recv용으로 생성함
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)		// 데이터를 인자로 주고 생성하면 send용으로 생성함
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

	unordered_set<int> view_list; // 시야 처리를 위한 섹터 컨테이너. 아이디를 넣고 빼는 작업이 자주 있으므로 set이 가장 적절하며, 정렬할 필요가 없으므로 unordered로 사용한다.
	mutex _vl;


public:
	SESSION()
	{
		_socket = 0;
		_id = -1;
		x = y = 0;
		_name[0] = 0;		// 이름은 클라이언트가 보내주지 않으면 알 수가 없음
		//in_use = false;		// 현재 세션이 사용중인지를 보여주는 bool 변수(사용중이면 true, 빈차면 false)
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}		// 전역 변수로 생성한 배열은 프로그램 종료시까지 소멸자가 호출되지 않는다.

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;	// prev_remain이 있는만큼 버퍼의 크기가 줄어들어야 잘린 데이터를 이어받을 수 있다.
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };	// 패킷을 인자로 한 send용 overalpped 구조체 생성
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()	// 클라이언트의 로그인 정보 패킷 전달
	{
		SC_LOGIN_INFO_PACKET p;		// 로그인 정보가 담긴 패킷. 이 패킷에 대한 프로토콜은 서버와 클라이언트가 공유한다.
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

array<SESSION, MAX_USER> clients;	// clients 컨테이너는 게임 플레이 내내 거의 모든 함수에서 동시에 여러 스레드가 접근하므로 data race에 매우 위험
// 멀티스레드 프로그래밍에서 data race에 가장 안전한 컨테이너는 array이다.(벡터나 리스트보다 더 안전함)
// 단 그렇다한들 컨테이너가 안전한 거지 컨테이너 내부 SESSION의 원소를 접근할 땐 Data Race가 생길 수 있음 -> 뮤텍스를 쓴다.
SOCKET g_c_socket, g_s_socket;	// g_c_socket, g_s_socket, g_a_over 변수는 여러 스레드가 동시에 접근하지 않으므로 data race에 안전하다.
OVER_EXP g_a_over;


bool can_see(int from, int to)
{
	if (abs(clients[from].x - clients[to].x) > VIEW_RANGE)	// 시야 범위를 나타내는 변수 VIEW_RANGE를 프로토콜 헤더 4로 선언했음
		return false;
	return abs(clients[from].y - clients[to].y <= VIEW_RANGE);

}


void SESSION::send_move_packet(int c_id)	// 클라이언트의 움직임 정보 패킷 전달
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
	_vl.lock(); // 락
	if (view_list.count(c_id))	// 없는 요소를 erase하려고 하면 프로그램이 죽어버리니까 검사하고
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
	for (int i = 0; i < MAX_USER; ++i) // 컨테이너에서 빈 자리 탐색
	{
		lock_guard<mutex> ll{ clients[i].s_lock };	// 락 가드를 사용하면 return 등으로 도중에 함수가 종료돼도 락을 안전하게 해제함 
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;		// 꽉차면 접속 불가능
}

void process_packet(int c_id, char* packet)
{
	// 모든 패킷의 처리가 담겨있는 매우 중요한 함수
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);	// 패킷을 로그인 패킷으로 캐스팅
		strcpy_s(clients[c_id]._name, p->name); // 이름 복사
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id].s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {	// 기존에 접속중인 클라이언트들에게 이 클라이언트가 접속했다는 정보를 보내야 한다. 

			{
				lock_guard<mutex> ll{ pl.s_lock };
				if (pl._state != ST_INGAME) continue;	// 접속 안한 클라는 빼고
			}
			if (pl._id == c_id) continue;
			if (false == can_see(c_id, pl._id))	// 내 클라이언트가 그 클라이언트를 볼 수 없으면(시야 안에 없으면)
				continue;

			pl.send_add_player_packet(c_id);

		}
		break;
	}
	case CS_MOVE: {	// 입력을 받아 클라이언트의 위치를 바꾸는 함수
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
		unordered_set<int> old_vlist = clients[c_id].view_list;	// 최적화를 위한 뷰리스트
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
			cpl._vl.lock();	// view_list에 접근하는 것은 data race의 위험이 있으므로 락을 써야함
			if (cpl.view_list.count(c_id))	// 상대방의 view_list에 내 id가 있는지(날 보고 있는지) 체크 
			{
				cpl._vl.unlock();
				cpl.send_move_packet(c_id);
			}
			else {
				cpl._vl.unlock();
				cpl.send_add_player_packet(c_id);	// 없으면 날 뷰리스트에 추가해줘
			}

			if (old_vlist.count(pl) == 0)	// 만약 내 뷰리스트에 상대가 없으면 상대를 추가해주기 
				clients[c_id].send_add_player_packet(pl);	// 굳이 언락을 두번 쓰는 이유는 락을 한시라도 빨리 끄기 위해서임(성능)
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

void disconnect(int c_id)	// 클라이언트와의 연결을 끊는 함수
{
	clients[c_id]._vl.lock();
	unordered_set<int> vl = clients[c_id].view_list;
	clients[c_id]._vl.unlock();
	for (auto& p_id : vl) {	// 모든 클라이언트들을 미접속 상태로 되돌림
		auto& pl = clients[p_id];
		{
			lock_guard<mutex> ll{ pl.s_lock };
			if (pl._state != ST_INGAME) continue;	// 접속 안한 클라는 빼고
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
	while (true) {		// 서버 함수 메인 루프
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE); // IOCP에서 IO 완료를 대기하는 함수(INFINITE)
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";	// accept타입 구조체에서 에러가 발생한 경우 에러문구 출력
			else {
				// GQCS에 에러가 발생한 경우 disconnect로 연결을 끊어준다.
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {	// 아직 자리가 있으면 새로운 클라이언트 생성
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

				clients[client_id].do_recv();	// accept가 완료되면 즉시 recv를 걸어둠
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else { // 클라이언트 아이디가 더 없을 경우 가득찼다는 문구 출력
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {	// recv는 패킷을 재조립하여 올바르게 정리한 후 패킷에 맞는 프로세스를 수행한다.
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
			if (remain_data > 0) {	// 남아있는 데이터가 있을 경우
				memcpy(ex_over->_send_buf, p, remain_data);	// 버퍼에 저장해둔다.
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
		cout << "WSAStartup 실패\n";
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

	// Accept 하기 전에 서버 소켓을 IOCP에 등록해주어야 한다.
	// CreateIoCompletionPort에는 두 가지 기능이 있다.
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);						// 1. INVALID_HANDLE_VALUE를 인자로 주어 IOCP 핸들을 생성 후 리턴
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);		// 2. 디바이스 핸들을 제공하여 IOCP와 디바이스를 연결. 연동된 핸들 리턴
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;	// accept 타입만 설정함. WSA버퍼나 문자열 버퍼 같은 건 쓸 필요가 없음(accept만 하므로)

	// AcceptEx - 비동기 방식으로 작동하는 Accept 함수
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);


	// 멀티스레드를 위한 코드
	// GQCS가 스레드에서 실행되기 때문에 스레드마다 공유되는 데이터를 신경써야 한다.
	// 스레드가 공유하는 데이터를 전달하기 위해 대부분의 데이터가 전역변수화되었다.
	// 전역 변수가 생성되어 발생하는 data race를 주의해야 한다.


	vector<thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();	// 현재 CPU의 가용 코어 수
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();




	closesocket(g_s_socket);
	WSACleanup();
}