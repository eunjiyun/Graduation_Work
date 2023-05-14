#pragma once
#include "stdafx.h"
#include "MemoryPool.h"

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
    short view_range, type;
    array<float, 3> distances = { 10000.f };
    NPC_State curState = NPC_State::Idle;
    //AStar_Pool _Pool;
public:

    float g_distance = 0;
    XMFLOAT3 MagicPos = { 5000, 5000, 5000 };
    XMFLOAT3 MagicLook = { 0, 0, 0 };
    high_resolution_clock::time_point recent_recvedTime;
    bool alive = false;

    short HP, power;
    float speed;
    BoundingBox BB;
    XMFLOAT3 Pos;
    short cur_animation_track = 0;
    float attack_cycle = 0.f;
    float attack_range = 0.f;
    short room_num; // 이 몬스터 객체가 존재하는 게임 룸 넘버
    short target_id = -1; // 추적하는 플레이어 ID
    short m_id = -1;    // 몬스터 자체ID
    float dead_timer = 0;
    float attack_timer = 0;
    bool attacked = false;
    mutable mutex m_lock; // const 함수에서 lock을 사용하기 위해 mutable로 선언
    Monster() { }
    Monster(const Monster& other);
    ~Monster();
    Monster& operator=(const Monster& other);
    void Initialize(short _roomNum, short _id, short _type, XMFLOAT3 _pos);
    short getType()
    {
        return type;
    }

    int get_targetID();
    XMFLOAT3 Find_Direction(float fTimeElapsed, XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos);
    void Update(float fTimeElapsed);
    XMFLOAT3 GetPosition() { return Pos; }
    float GetSpeed() { return speed; }
    short GetPower() { return power; }
    void SetState(NPC_State st) { curState = st; }
    NPC_State GetState() const { return curState; }
    void SetAttackTimer(float time) { attack_timer = time; }
    float GetAttackTimer() const { return attack_timer; }
    //bool is_alive() { return alive.load(); }
    //void SetAlive(bool in) { alive.store(in); }
    bool check_path(const XMFLOAT3& _pos, unordered_set<XMFLOAT3, XMFLOAT3Hash, XMFLOAT3Equal>& CloseList, BoundingBox& check_box);
};

class SorcererMonster : public Monster
{
public:
    XMFLOAT3 MagicPos = { 5000, 5000, 5000 };
    XMFLOAT3 MagicLook;
    void Update(float fTimeElapsed);
};
class MonsterInfo
{
public:
    XMFLOAT3 Pos;
    short type;
    short id;
    MonsterInfo(XMFLOAT3 _pos, short _type, int _id) : Pos(_pos), type(_type), id(_id) {}
};




