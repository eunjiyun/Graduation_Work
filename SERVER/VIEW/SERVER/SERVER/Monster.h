#pragma once
#include "stdafx.h"
#include <mutex>


enum class NPC_State
{
    Idle,
    Chase,
    Attack,
    Dead
};

class Monster
{
private:
    XMFLOAT3 m_xmf3Look = { 0, 0, 1 };
    XMFLOAT3 m_xmf3Up = { 0, 1, 0 };
    XMFLOAT3 m_xmf3Right = { 1, 0, 0 };
    short view_range, type;
    array<float, 4> distances = { 10000.f };
    NPC_State curState = NPC_State::Idle;
public:
    bool Move_Lock = false;
    stack<XMFLOAT3> roadToMove;
    short HP, power, speed;
    BoundingBox BB;
    bool is_alive = false;
    XMFLOAT3 Pos;
    short cur_animation_track = 0;
    double attack_cycle = 2.35;
    double attack_timer = 1.f;
    short room_num; // 이 몬스터 객체가 존재하는 게임 룸 넘버
    short target_id = -1; // 추적하는 플레이어 ID
    short m_id = -1;    // 몬스터 자체ID
    mutable mutex m_lock;
    Monster() { }
    Monster(const Monster& other);
    Monster& operator=(const Monster& other);
    void Initialize(short _roomNum, short _id, short _type, XMFLOAT3 _pos);
    short getType()
    {
        return type;
    }
    void Move(XMFLOAT3 m_Shift);

    int get_targetID();
    XMFLOAT3 Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos);
    void Update(float fTimeElapsed);
    XMFLOAT3 GetPosition() { return Pos; }
    XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
    XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
    XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
    short GetSpeed() { return speed; }
    short GetPower() { return power; }
    void SetState(NPC_State st) { curState = st; }
};

class MonsterInfo
{
public:
    XMFLOAT3 Pos;
    short type;
    short id;
    MonsterInfo(XMFLOAT3 _pos, short _type, int _id) : Pos(_pos), type(_type), id(_id) {}
};





