#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include "protocol.h"

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
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
	int _id;
	SOCKET _socket;
	XMFLOAT3 m_xmf3Position, m_xmf3Look, m_xmf3Up, m_xmf3Right, m_xmf3Velocity, m_xmf3Gravity;
	float m_fPitch, m_fYaw, m_fRoll;
	float m_fMaxVelocityXZ, m_fMaxVelocityY, m_fFriction;
	DWORD direction;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	BoundingBox m_xmOOBB;
	short error_stack;
	bool onAttack, onCollect, onDie, onRun;
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
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
		m_xmOOBB = BoundingBox(m_xmf3Position, XMFLOAT3(10, 3, 10));
		error_stack = 0;
		onAttack = false;
		onCollect = false;
		onDie = false;
		onRun = false;
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
		p.x = m_xmf3Position.x;
		p.y = m_xmf3Position.y;
		p.z = m_xmf3Position.z;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
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
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
		m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	}
	void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);


		if (dwDirection & DIR_ATTACK) onAttack = true; else onAttack = false;
		if (dwDirection & DIR_DIE) onDie = true; else onDie = false;
		if (dwDirection & DIR_COLLECT) onCollect = true; else onCollect = false;


		if (dwDirection & DIR_RUN) onRun = true; else onRun = false;
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		Move(xmf3Shift, bUpdateVelocity);




	}

	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
	{
		if (bUpdateVelocity)
		{
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		}
		else
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
			//if (m_pCamera) m_pCamera->Move(xmf3Shift);
		}
	}

	void UpdateBoundingBox()
	{
		m_xmOOBB.Center = m_xmf3Position;
	}

	void Update(float fTimeElapsed)
	{
		Move(direction, 21.0f, true);

		if (onAttack || onCollect || onDie) m_xmf3Velocity = { 0, 0, 0 };
		
		if (onRun) m_fMaxVelocityXZ = 100.0f; else m_fMaxVelocityXZ = 10.0f;


		
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		float fMaxVelocityXZ = m_fMaxVelocityXZ;
		if (fLength > m_fMaxVelocityXZ)
		{
			m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
			m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
		}

		

		XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
		Move(xmf3Velocity, false);

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
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void CheckPosition(XMFLOAT3 newPos);
	void CheckCollision(float fTimeElapsed);
};

