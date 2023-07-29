#pragma once
#include "stdafx.h"
#include "MemoryPool.h"

#define MONSTER_SIZE XMFLOAT3{15,15,15}
#define SMALL_MONSTER_SIZE XMFLOAT3{7,7,7}
#define BOSS_MONSTER_SIZE XMFLOAT3{30,30,30}
#define MELEE_ATTACK_RANGE 20
#define MAGIC_ATTACK_RANGE 150
#define BOSS_ATTACK_RANGE 75

enum class NPC_State
{
    Idle,
    Chase,
    Attack,
    Dead
};

struct Player_Distance
{
    short _id;
    float distance;
};
class Monster
{
private:
    short type;
    array<Player_Distance, 3> distances;
    NPC_State curState = NPC_State::Idle;
public:
    //AStar_Pool* m_pool;
    float g_distance = 0;
    XMFLOAT3 MagicPos = { 5000, 5000, 5000 };
    XMFLOAT3 MagicLook = { 0, 0, 0 };
    high_resolution_clock::time_point recent_updateTime;
    atomic_bool alive = false;

    short HP, power, view_range;
    float speed;
    BoundingBox BB;
    XMFLOAT3 Pos;
    float attack_cycle = 0.f;
    float attack_range = 0.f;
    short room_num; // 이 몬스터 객체가 존재하는 게임 룸 넘버
    short target_id = -1; // 추적하는 플레이어 ID
    short m_id = -1;    // 몬스터 자체ID
    float dead_timer = 0;
    float attack_timer = 0;
    float wander_timer = 0;
    bool attacked = false;
    bool wander = false;
    mutable mutex m_lock; // const 함수에서 lock을 사용하기 위해 mutable로 선언
    Monster() { }
    Monster(const Monster& other);
    ~Monster();
    Monster& operator=(const Monster& other);
    void Initialize(short _roomNum, short _id, short _type, XMFLOAT3 _pos);
    void Re_Initialize(short _type, XMFLOAT3 _pos);
    short getType()
    {
        return type;
    }
    void setType(int _type)
    {
        type = _type;
    }

    int get_targetID();
    XMFLOAT3 Find_Direction(float fTimeElapsed, XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos);
    virtual void Update(float fTimeElapsed);
    XMFLOAT3 GetPosition() { return Pos; }
    float GetSpeed() { return speed; }
    short GetPower() { return power; }
    void SetState(NPC_State st) { curState = st; }
    NPC_State GetState() const { return curState; }
    void SetAttackTimer(float time) { attack_timer = time; }
    float GetAttackTimer() const { return attack_timer; }
    bool check_path(const XMFLOAT3& _pos, unordered_set<XMFLOAT3, PointHash, PointEqual>& CloseList, BoundingBox& check_box);
};

class SorcererMonster : public Monster
{
public:
    void Update(float fTimeElapsed) override;
};

class BossMonster : public Monster
{
public:
    void Update(float fTimeElapsed) override;
};

class MonsterInfo
{
public:
    XMFLOAT3 Pos;
    short type;
    short id;
    MonsterInfo(XMFLOAT3 _pos, short _type, int _id) : Pos(_pos), type(_type), id(_id) {}
};




