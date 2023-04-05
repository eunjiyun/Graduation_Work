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

    return *this;
}

void Monster::Initialize(short _roomNum, short _id, short _type, XMFLOAT3 _pos)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.0f, 0.5f);
    Pos = _pos;
    room_num = _roomNum;
    alive = true;
    m_id = _id;
    BB = BoundingBox(_pos, XMFLOAT3(5, 3, 5));
    curState = NPC_State::Idle;
    switch (_type)
    {
    case 0: // º’ø° ƒÆ
        type = 0;
        HP = 200;
        power = 30;
        view_range = 500;
        speed = 1.75f + dis(gen);
        attack_cycle = (71.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 1: // ª¿¥Ÿ±Õ ¥Ÿ∏Æ
        type = 1;
        HP = 100;
        power = 30;
        view_range = 500;
        speed = 2.75f + dis(gen);
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 2: // ±ÕΩ≈
        type = 2;
        HP = 50;
        power = 50;
        view_range = 500;
        speed = 2.75f + dis(gen);
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 3:
        type = 3;
        HP = 50;
        power = 70;
        view_range = 400;
        speed = 1.75f + dis(gen);
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 4: // ∏∂º˙ªÁ
        type = 4;
        HP = 50;
        power = 70;
        view_range = 400;
        speed = 1.75f + dis(gen);
        attack_cycle = (8.f / 3.f); // 2.666664√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    case 5:
        type = 5;
        HP = 500;
        power = 70;
        view_range = 400;
        speed = 1.75f + dis(gen);
        attack_cycle = (56.f / 30.f); // 2.366667√ 
        attack_timer = attack_cycle;
        dead_timer = 3.3f;
        break;
    }
}

void Monster::Move(XMFLOAT3 m_Shift)
{
    Pos = Vector3::Add(Pos, m_Shift);
    BB.Center = Pos;
}
