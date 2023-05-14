#pragma once

#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include "protocol.h"
#include "Monster.h"
#include "MemoryPool.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")


enum COMP_TYPE { OP_ACCEPT, OP_LOGGEDIN, OP_RECV, OP_SEND, OP_NPC_MOVE };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};


class OVERLAPPEDPOOL {
private:
	concurrent_queue<OVER_EXP*> objectQueue;
public:
	OVERLAPPEDPOOL(size_t MemorySize)
	{
		for (int i = 0; i < MemorySize; ++i) {
			objectQueue.push(new OVER_EXP());
		}
	}
	~OVERLAPPEDPOOL()
	{
		OVER_EXP* o;
		while (objectQueue.try_pop(o)) {
			unique_ptr<OVER_EXP> pData(o);
		}
	}

	OVER_EXP* GetMemory()
	{
		OVER_EXP* mem = nullptr;

		if (objectQueue.empty()) {
			for (int i = 0; i < 5000; ++i)
				objectQueue.push(new OVER_EXP());
		}

		objectQueue.try_pop(mem);

		return mem;
	}

	OVER_EXP* GetMemory(char* packet)
	{
		OVER_EXP* mem = nullptr;

		if (objectQueue.empty()) {
			cout << "OverlappedPool called add memory request\n";
			for (int i = 0; i < 5000; ++i)
				objectQueue.push(new OVER_EXP());
		}
		objectQueue.try_pop(mem);

		mem->_wsabuf.len = packet[0];
		//mem->_wsabuf.buf = mem->_send_buf;
		//ZeroMemory(&mem->_over, sizeof(mem->_over));
		mem->_comp_type = OP_SEND;
		memcpy(mem->_send_buf, packet, packet[0]);
		return mem;
	}
	void ReturnMemory(OVER_EXP* Mem)
	{
		ZeroMemory(&Mem->_over, sizeof(Mem->_over));
		Mem->_wsabuf.len = BUF_SIZE;
		Mem->_wsabuf.buf = Mem->_send_buf;
		Mem->_comp_type = OP_RECV;
		objectQueue.push(Mem);
	}
	void PrintSize()
	{
		cout << "CurrentSize - " << objectQueue.unsafe_size() << endl;
	}
};



//OVERLAPPEDPOOL OverPool(500'000);

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME, ST_DEAD, ST_CRASHED };
class SESSION {
	OVER_EXP _recv_over;
public:
	mutex _s_lock;
	atomic<S_STATE> _state;
	short _id;
	SOCKET _socket;
	XMFLOAT3 m_xmf3Position, m_xmf3Look, m_xmf3Up, m_xmf3Right, m_xmf3Velocity; 
	float HP;
	atomic<DWORD> direction;
	//char	_name[NAME_SIZE];
	unsigned short	_prev_remain;
	BoundingBox m_xmOOBB;
	atomic<short> cur_stage;
	short error_stack;
	short character_num;
	int recent_recvedTime;
	float clear_percentage;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		m_xmf3Position = { 0.f,0.f,0.f };
		m_xmf3Velocity = { 0.f,0.f,0.f };
		m_xmf3Look = { 0.f,0.f,1.f };
		m_xmf3Up = { 0.f,1.f,0.f };
		m_xmf3Right = { 1.f,0.f,0.f };
		direction = 0;
		cur_stage = 0;
		//_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
		m_xmOOBB = BoundingBox(m_xmf3Position, XMFLOAT3(15, 10, 12));
		error_stack = 0;
		character_num = 0;
		HP = 0;
		clear_percentage = 0.f;
	}

	~SESSION() {}

	void Initialize()
	{
		//_id = id;
		m_xmf3Position = XMFLOAT3{ 300 + 50.f * (_id % 3), -50,600 };// 중간발표 데모를 위해 시작위치를 임의로 조정  //-259,4500
		m_xmf3Velocity = { 0.f,0.f,0.f };
		direction = 0;
		_prev_remain = 0;
		m_xmf3Up = { 0,1,0 };
		m_xmf3Right = { 1,0,0 };
		m_xmf3Look = { 0,0,1 };
		//_socket = Socket;
		cur_stage = 0;
		error_stack = 0;
		character_num = 0;
		HP =  55500;
		clear_percentage = 1.f; // 중간발표 데모를 위해 시작위치를 임의로 조정
	}
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		int ret = WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
		//if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING) err_display("WSARecv()");
	}

	void do_send(void* packet)
	{		
		//OVER_EXP* sdata = OverPool.GetMemory(reinterpret_cast<char*>(packet));
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		int ret = WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
		//if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING) err_display("WSASend()");
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.pos = m_xmf3Position;
		do_send(&p);
	}
	void send_move_packet(SESSION* Player)
	{
		SC_MOVE_PLAYER_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(SC_MOVE_PLAYER_PACKET);
		p.type = SC_MOVE_PLAYER;

		p.Pos = Player->GetPosition();
		p.direction = Player->direction.load();
		p.HP = Player->HP;
		p.vel = Player->GetVelocity();
#ifdef _STRESS_TEST
		p.move_time = Player->recent_recvedTime;
#endif
		do_send(&p);
	}

	void send_rotate_packet(SESSION* Player)
	{
		SC_ROTATE_PLAYER_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(SC_ROTATE_PLAYER_PACKET);
		p.type = SC_ROTATE_PLAYER;
		p.Look = Player->GetLookVector();
		p.Right = Player->GetRightVector();
		do_send(&p);
	}
	void send_attack_packet(SESSION* Player)
	{
		CS_ATTACK_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(CS_ATTACK_PACKET);
		p.type = CS_ATTACK;
		do_send(&p);
	}

	void send_interaction_packet(int _stage_id, int _obj_id)
	{
		SC_INTERACTION_PACKET p;
		p.stage_id = _stage_id;
		p.obj_id = _obj_id;
		p.size = sizeof(SC_INTERACTION_PACKET);
		p.type = SC_INTERACTION;
		do_send(&p);
	}

	void send_changeweapon_packet(SESSION* Player)
	{
		CS_CHANGEWEAPON_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(CS_CHANGEWEAPON_PACKET);
		p.type = CS_CHANGEWEAPON;
		p.cur_weaponType = Player->character_num;
		do_send(&p);
	}

	void send_add_player_packet(SESSION* Player)
	{
		SC_ADD_PLAYER_PACKET add_packet;
		add_packet.id = Player->_id;

		add_packet.size = sizeof(SC_ADD_PLAYER_PACKET);
		add_packet.type = SC_ADD_PLAYER;
		add_packet.cur_weaponType = Player->character_num;
		add_packet.Look = Player->m_xmf3Look;
		add_packet.Right = Player->m_xmf3Right;
		add_packet.Up = Player->m_xmf3Up;
		add_packet.Pos = Player->GetPosition();
		do_send(&add_packet);
	}


	void send_summon_monster_packet(Monster* M);
	void send_NPCUpdate_packet(Monster* M);
	void send_open_door_packet(int door_num)
	{
		SC_OPEN_DOOR_PACKET packet;
		packet.size = sizeof(SC_OPEN_DOOR_PACKET);
		packet.type = SC_OPEN_DOOR;
		packet.door_num = door_num;
		do_send(&packet);
	}

	void send_clear_packet()
	{
		SC_GAME_CLEAR_PACKET packet;
		packet.size = sizeof(SC_GAME_CLEAR_PACKET);
		packet.type = SC_GAME_CLEAR;
		do_send(&packet);
	}

	void send_remove_player_packet(int c_id)
	{
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(SC_REMOVE_PLAYER_PACKET);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	}
	void Rotate(float x, float y, float z)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(DirectX::XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
		//m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	}

	void Move(const XMFLOAT3& xmf3Shift)
	{	
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);	
	}

	void UpdateBoundingBox()
	{
		m_xmOOBB.Center = m_xmf3Position;
		m_xmOOBB.Center.y += 10.f;
	}

	XMFLOAT3 GetReflectVec(XMFLOAT3 ObjLook, XMFLOAT3 MovVec)
	{
		float Dot = Vector3::DotProduct(MovVec, ObjLook);
		XMFLOAT3 Nor = Vector3::ScalarProduct(ObjLook, Dot, false);
		XMFLOAT3 SlidingVec = Vector3::Subtract(MovVec, Nor);
		return SlidingVec;
	}

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z)); }
	const XMFLOAT3& GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void Update();
	void CheckPosition(XMFLOAT3 newPos);
};

