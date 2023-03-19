#include "Monster.h"

void Monster::Initialize(short _roomNum, short _type, XMFLOAT3 _pos)
{
    Pos = _pos;
    room_num = _roomNum;
    is_alive = true;
    BB = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(5, 3, 5));

    switch (_type)
    {
    case 0: // ¼Õ¿¡ Ä®
        type = 0;
        HP = 200;
        power = 30;
        view_range = 500;
        speed = 2;
        break;
    case 1: // »À´Ù±Í ´Ù¸®
        type = 1;
        HP = 100;
        power = 30;
        view_range = 500;
        speed = 3;
        break;
    case 2: // ±Í½Å
        type = 2;
        HP = 50;
        power = 50;
        view_range = 500;
        speed = 4;
        break;
    case 3:
        type = 3;
        HP = 50;
        power = 70;
        view_range = 400;
        speed = 2;
        break;
    case 4: // ¸¶¼ú»ç
        type = 4;
        HP = 50;
        power = 70;
        view_range = 400;
        speed = 2;
        break;
    case 5:
        type = 5;
        HP = 500;
        power = 70;
        view_range = 400;
        speed = 2;
        break;
    }
}

void Monster::Move(XMFLOAT3 m_Shift)

{
    Pos = Vector3::Add(Pos, m_Shift);
    BB.Center = Pos;
}
