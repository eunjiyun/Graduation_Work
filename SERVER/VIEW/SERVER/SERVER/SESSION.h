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


enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_UPDATE };
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

enum S_STATE { ST_FREE, ST_ALLOC, ST_CRASHED, ST_INGAME, ST_DEAD };
enum WEAPON_TYPE {BLADE, GUN, PUNCH};
class SESSION {
	OVER_EXP _recv_over;
public:
	mutex _s_lock;
	atomic<S_STATE> _state;
	short _id;
	wchar_t _name[IDPW_SIZE]{};
	SOCKET _socket;
	XMFLOAT3 m_xmf3Position, m_xmf3Look, m_xmf3Up, m_xmf3Right, m_xmf3Velocity; 
	float HP;
	atomic<DWORD> direction;
	unsigned short	_prev_remain;
	BoundingBox m_xmOOBB;
	atomic<short> cur_stage;
	WEAPON_TYPE weapon_type;
	int recent_updateTime;
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
		_state = ST_FREE;
		_prev_remain = 0;
		m_xmOOBB = BoundingBox(m_xmf3Position, XMFLOAT3(15, 10, 15));
		weapon_type = BLADE;
		HP = 0;
		clear_percentage = 0.f;
	}

	~SESSION() {}

	void Initialize()
	{
		//_id = id;
		m_xmf3Position = XMFLOAT3{ 300 + 50.f * (_id % 3), -63,600 };// 중간발표 데모를 위해 시작위치를 임의로 조정  //-259,4500
		m_xmf3Velocity = { 0.f,0.f,0.f };
		direction = 0;
		_prev_remain = 0;
		m_xmf3Up = { 0,1,0 };
		m_xmf3Right = { 1,0,0 };
		m_xmf3Look = { 0,0,1 };
		//_socket = Socket;
		cur_stage = 0;
		weapon_type = BLADE;
		HP =  5000;
		clear_percentage = 0.f; // 중간발표 데모를 위해 시작위치를 임의로 조정
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
	void send_game_start_packet()
	{
		SC_GAME_START_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_GAME_START_PACKET);
		p.type = SC_GAME_START;
		p.pos = m_xmf3Position;
		do_send(&p);
	}
	void send_update_packet(SESSION* Player)
	{
		SC_UPDATE_PLAYER_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(SC_UPDATE_PLAYER_PACKET);
		p.type = SC_UPDATE_PLAYER;
		p.Pos = Player->GetPosition();
		p.direction = Player->direction.load();
		p.HP = Player->HP;
		p.vel = Player->GetVelocity();
#ifdef _STRESS_TEST
		p.move_time = Player->recent_updateTime;
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
		SC_ATTACK_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(SC_ATTACK_PACKET);
		p.type = SC_ATTACK;
		//p.damaged_monster_id = damaged_monster_id;
		do_send(&p);
	}

	void send_monster_damaged_packet(int player_id, int monster_id, int monster_HP)
	{
		SC_MONSTER_DAMAGED_PACKET p;
		p.size = sizeof(SC_MONSTER_DAMAGED_PACKET);
		p.type = SC_MONSTER_DAMAGED;
		p.monster_id = monster_id;
		p.player_id = player_id;
		p.remain_HP = monster_HP;
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
		SC_CHANGEWEAPON_PACKET p;
		p.id = Player->_id;
		p.size = sizeof(SC_CHANGEWEAPON_PACKET);
		p.type = SC_CHANGEWEAPON;
		p.cur_weaponType = Player->weapon_type;
		do_send(&p);
	}

	void send_add_player_packet(SESSION* Player)
	{
		SC_ADD_PLAYER_PACKET add_packet;
		add_packet.id = Player->_id;

		add_packet.size = sizeof(SC_ADD_PLAYER_PACKET);
		add_packet.type = SC_ADD_PLAYER;
		add_packet.Pos = Player->GetPosition();

		cout << add_packet.id << " - ";
		Vector3::Print(add_packet.Pos);
		do_send(&add_packet);
	}


	void send_summon_monster_packet(Monster* M)
	{
		SC_SUMMON_MONSTER_PACKET summon_packet;
		summon_packet.id = M->m_id;
		summon_packet.size = sizeof(summon_packet);
		summon_packet.type = SC_SUMMON_MONSTER;
		summon_packet.Pos = M->GetPosition();
#ifdef _STRESS_TEST
		summon_packet.room_num = M->room_num;
#endif
		summon_packet.monster_type = M->getType();
		do_send(&summon_packet);
	}

	void send_monster_update_packet(Monster* M)
	{
		SC_MOVE_MONSTER_PACKET p;
		p.id = M->m_id;
		p.size = sizeof(SC_MOVE_MONSTER_PACKET);
		p.type = SC_MOVE_MONSTER;
		p.target_id = M->target_id;
		p.Pos = M->GetPosition();
		p.HP = M->HP;
		p.is_alive = M->alive;
		p.animation_track = (short)M->GetState();
		p.BulletPos = M->MagicPos;

#ifdef _STRESS_TEST
		p.room_num = M->room_num;
#endif
		do_send(&p);
	}


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

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z)); }
	const XMFLOAT3& GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void Update(CS_HEARTBEAT_PACKET* packet);
	void CheckPosition(XMFLOAT3 newPos);
};

