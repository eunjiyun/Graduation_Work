#include "Monster.h"



Monster::Monster(const Monster& other)
{
    lock_guard<mutex> ll{ other.m_lock };
    Pos = other.Pos;
    room_num = other.room_num;
    alive = other.alive;
    BB = other.BB;
    m_id = other.m_id;
    type = other.type;
    HP = other.HP;
    power = other.power;
    view_range = other.view_range;
    speed = other.speed;
    curState = other.curState;
    recent_recvedTime = other.recent_recvedTime;
    distances = other.distances;
    cur_animation_track = other.cur_animation_track;
    target_id = other.target_id;
    attacked = other.attacked;
}

Monster& Monster::operator=(const Monster& other)
{
    if (this == &other) {
        return *this; 
    }
    lock_guard<mutex> l1{ this->m_lock };
    lock_guard<mutex> l2{ other.m_lock };
    Pos = other.Pos;
    room_num = other.room_num;
    alive = other.alive;
    BB = other.BB;
    m_id = other.m_id;
    type = other.type;
    HP = other.HP;
    power = other.power;
    view_range = other.view_range;
    speed = other.speed;
    curState = other.curState;
    recent_recvedTime = other.recent_recvedTime;
    distances = other.distances;
    cur_animation_track = other.cur_animation_track;
    target_id = other.target_id;
    attacked = other.attacked;
    return *this;
}

Monster::~Monster()
{

}
void Monster::Initialize(short _roomNum, short _id, short _type, XMFLOAT3 _pos) 
{
    Pos = _pos;
    room_num = _roomNum;
    alive = true;
    m_id = _id;
    BB = BoundingBox(_pos, XMFLOAT3(15, 20, 12));
    curState = NPC_State::Idle;
    recent_recvedTime = high_resolution_clock::now();
    distances = { 10000.f };
    cur_animation_track = 0;
    target_id = -1;
    attacked = false;
    attack_range = 5.f;
    switch (_type)
    {
    case 0: // º’ø° ƒÆ
        type = 0;
        HP = 250;
        power = 30;
        view_range = 500;
        speed = 40.f;
        attack_cycle = (71.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 1: // ª¿¥Ÿ±Õ ¥Ÿ∏Æ
        type = 1;
        HP = 150;
        power = 30;
        view_range = 500;
        speed = 50.f;
        attack_range = 3.f;
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 2: // ∏∂º˙ªÁ
        type = 2;
        HP = 50;
        power = 70;
        view_range = 500;
        speed = 30.f;
        attack_range = 150.f;
        attack_cycle = (8.f / 3.f); // 2.666664√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 3:
        type = 3;
        HP = 50;
        power = 70;
        view_range = 300;
        speed = 50.f;
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 4: // ±ÕΩ≈
        type = 4;
        HP = 50;
        power = 50;
        view_range = 300;
        speed = 50.f;
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 5:
        type = 5;
        HP = 500;
        power = 70;
        view_range = 300;
        speed = 50.f;
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    }
}

