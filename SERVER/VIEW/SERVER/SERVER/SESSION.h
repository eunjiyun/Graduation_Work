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


enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE };
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
	queue<OVER_EXP*> objectQueue;
	mutex pool_lock;
public:
	OVERLAPPEDPOOL(size_t MemorySize)
	{
		for (int i = 0; i < MemorySize; ++i) {
			objectQueue.push(new OVER_EXP());
		}
	}
	~OVERLAPPEDPOOL() {} // 풀이 소멸되면 서버가 그냥 끝난 거

	OVER_EXP* GetMemory()
	{
		OVER_EXP* mem = nullptr;
		{
			lock_guard<mutex> ll{ pool_lock };
			if (!objectQueue.empty()) {
				mem = objectQueue.front();
				objectQueue.pop();
			}
		}
		if (mem == nullptr) {
			throw runtime_error("FAILED TO ALLOCATE OVER_EXP IN POOL\n");
		}
		mem->_wsabuf.len = BUF_SIZE;
		mem->_wsabuf.buf = mem->_send_buf;
		ZeroMemory(&mem->_over, sizeof(mem->_over));
		return mem;
	}

	OVER_EXP* GetMemory(char* packet)
	{
		OVER_EXP* mem = nullptr;
		{
			lock_guard<mutex> ll{ pool_lock };
			if (!objectQueue.empty()) {
				mem = objectQueue.front();
				objectQueue.pop();
			}
		}
		if (mem == nullptr) {
			throw runtime_error("FAILED TO ALLOCATE OVER_EXP IN POOL\n");
		}
		mem->_wsabuf.len = packet[0];
		mem->_wsabuf.buf = mem->_send_buf;
		ZeroMemory(&mem->_over, sizeof(mem->_over));
		mem->_comp_type = OP_SEND;
		memcpy(mem->_send_buf, packet, packet[0]);
		return mem;
	}
	void ReturnMemory(OVER_EXP* Mem)
	{
		lock_guard<mutex> ll{ pool_lock };
		objectQueue.push(Mem);
	}
	void PrintSize()
	{
		cout << "CurrentSize - " << objectQueue.size() << endl;
	}
};



//CObjectPool<OVER_EXP> OverPool(100'000);
OVERLAPPEDPOOL OverPool(200'000);

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME, ST_DEAD };
class SESSION {
	OVER_EXP _recv_over;
public:
	mutex _s_lock;
	S_STATE _state;
	short _id;
	SOCKET _socket;
	XMFLOAT3 m_xmf3Position, m_xmf3Look, m_xmf3Up, m_xmf3Right, m_xmf3Velocity; 
	float HP;
	DWORD direction;
	char	_name[NAME_SIZE];
	unsigned short	_prev_remain;
	BoundingBox m_xmOOBB;
	short cur_stage;
	short error_stack;
	short character_num;
	high_resolution_clock::time_point recent_recvedTime;
	XMFLOAT3 BulletPos, BulletLook;
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
		BulletPos = { 5000,5000,5000 };
		direction = 0;
		cur_stage = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
		m_xmOOBB = BoundingBox(m_xmf3Position, XMFLOAT3(10, 4, 10));
		error_stack = 0;
		character_num = 0;
		HP = 10000;
	}

	~SESSION() {}

	void Initialize(int id, SOCKET Socket)
	{
		_id = id;
		m_xmf3Position = XMFLOAT3{ -50, -0, 590 };
		direction = 0;
		_prev_remain = 0;
		m_xmf3Up = XMFLOAT3{ 0,1,0 };
		m_xmf3Right = XMFLOAT3{ 1,0,0 };
		m_xmf3Look = XMFLOAT3{ 0,0,1 };
		_socket = Socket;
		cur_stage = 0;
		error_stack = 0;
		recent_recvedTime = high_resolution_clock::now();
	}
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		int ret = WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
		if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING) err_display("WSARecv()");
	}

	void do_send(void* packet)
	{		
		OVER_EXP* sdata = OverPool.GetMemory(reinterpret_cast<char*>(packet));
		//OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		int ret = WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
		if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING) err_display("WSASend()");
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

		p.Look = Player->GetLookVector();
		p.Right = Player->GetRightVector();
		p.Up = Player->GetUpVector();
		p.Pos = Player->GetPosition();
		p.direction = Player->direction;
		p.HP = Player->HP;
		p.BulletPos = Player->BulletPos;
		p.vel = Player->GetVelocity();
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

	void send_collect_packet(SESSION* Player)
	{
		CS_COLLECT_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(CS_COLLECT_PACKET);
		p.type = CS_COLLECT;
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

		strcpy_s(add_packet.name, Player->_name);
		add_packet.size = sizeof(add_packet);
		add_packet.type = SC_ADD_PLAYER;
		add_packet.Look = Player->m_xmf3Look;
		add_packet.Right = Player->m_xmf3Right;
		add_packet.Up = Player->m_xmf3Up;
		add_packet.Pos = Player->GetPosition();
		do_send(&add_packet);
	}


	void send_summon_monster_packet(Monster* M);
	void send_NPCUpdate_packet(Monster* M);


	void send_remove_player_packet(int c_id)
	{
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
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
		m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	}
	//void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
	//{
	//	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	//	onAttack = dwDirection & DIR_ATTACK && !onAttack;
	//	onDie = dwDirection & DIR_DIE && !onDie;
	//	onCollect = dwDirection & DIR_COLLECT && !onCollect;
	//	onRun = dwDirection & DIR_RUN && !onRun;
	//	onChange = dwDirection & DIR_CHANGESTATE && !onChange;
	//	character_num = (character_num + onChange) % 3;
	//}

	void Move(const XMFLOAT3& xmf3Shift)
	{
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		}
	}

	void UpdateBoundingBox()
	{
		m_xmOOBB.Center = m_xmf3Position;
	}

	XMFLOAT3 GetReflectVec(XMFLOAT3 ObjLook, XMFLOAT3 MovVec)
	{
		float Dot = Vector3::DotProduct(MovVec, ObjLook);
		XMFLOAT3 Nor = Vector3::ScalarProduct(ObjLook, Dot, false);
		XMFLOAT3 SlidingVec = Vector3::Subtract(MovVec, Nor);
		return SlidingVec;
	}

	//void Deceleration(float fTimeElapsed)
	//{
	//	float fLength = Vector3::Length(m_xmf3Velocity);
	//	float fDeceleration = (m_fFriction * fTimeElapsed);
	//	if (fDeceleration > fLength)fDeceleration = fLength;
	//	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	//	UpdateBoundingBox();
	//}

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

