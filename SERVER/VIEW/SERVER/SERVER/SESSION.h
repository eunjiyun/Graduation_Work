#pragma once

#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <concurrent_priority_queue.h>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
//
//enum EVENT_TYPE { EV_RANDOM_MOVE };
//
//struct TIMER_EVENT {
//	int obj_id;
//	chrono::system_clock::time_point wakeup_time;
//	EVENT_TYPE event_id;
//	int target_id;
//	constexpr bool operator < (const TIMER_EVENT& L) const
//	{
//		return (wakeup_time > L.wakeup_time);
//	}
//};
//concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

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

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
	OVER_EXP _recv_over;
public:
	mutex _s_lock;
	S_STATE _state;
	short _id;
	SOCKET _socket;
	XMFLOAT3 m_xmf3Position, m_xmf3Look, m_xmf3Up, m_xmf3Right, m_xmf3Velocity, m_xmf3Gravity;
	float m_fPitch, m_fYaw, m_fRoll;
	float m_fMaxVelocityXZ, m_fMaxVelocityY, m_fFriction;
	DWORD direction;
	char	_name[NAME_SIZE];
	unsigned short	_prev_remain;
	BoundingBox m_xmOOBB;
	short cur_stage;
	short error_stack;
	bool onAttack, onCollect, onDie, onRun;
	short character_num;
	//int		_last_move_time;
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
		m_xmf3Gravity = { 0.f, -250.f, 0.f };
		m_fPitch = m_fYaw = m_fRoll = 0.f;
		m_fMaxVelocityY = 100.f;
		m_fMaxVelocityXZ = 10.f;
		m_fFriction = 20.f;
		direction = 0;
		cur_stage = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
		m_xmOOBB = BoundingBox(m_xmf3Position, XMFLOAT3(10, 3, 10));
		error_stack = 0;
		onAttack = false;
		onCollect = false;
		onDie = false;
		onRun = false;
		character_num = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
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
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_summon_monster_packet(int npc_id);
	void send_NPCUpdate_packet(int npc_id);

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
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
		m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	}
	void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		onAttack = dwDirection & DIR_ATTACK;
		onDie = dwDirection & DIR_DIE;
		onCollect = dwDirection & DIR_COLLECT;
		onRun = dwDirection & DIR_RUN;
		character_num = (character_num + (dwDirection & DIR_CHANGESTATE)) % 3;

		xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(Vector3::ScalarProduct(m_xmf3Look, fDistance, false), dwDirection& DIR_FORWARD, false));
		xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(Vector3::ScalarProduct(m_xmf3Look, -fDistance, false), dwDirection & DIR_BACKWARD, false));
		xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(Vector3::ScalarProduct(m_xmf3Right, fDistance, false), dwDirection & DIR_RIGHT, false));
		xmf3Shift = Vector3::Add(xmf3Shift, Vector3::ScalarProduct(Vector3::ScalarProduct(m_xmf3Right, -fDistance, false), dwDirection & DIR_LEFT, false));
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);



	}

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

	void Deceleration(float fTimeElapsed)
	{
		float fLength = Vector3::Length(m_xmf3Velocity);
		float fDeceleration = (m_fFriction * fTimeElapsed);
		if (fDeceleration > fLength)fDeceleration = fLength;
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

		UpdateBoundingBox();
	}

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z)); }
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void Update(float fTimeElapsed);
	void CheckPosition(XMFLOAT3 newPos);
	void CheckCollision(float fTimeElapsed);
};

