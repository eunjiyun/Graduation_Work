// protocol.h

constexpr short PORT_NUM = 3500;
constexpr short BUF_SIZE = 512;
constexpr short ID_SIZE = 10;
constexpr short PASSWORD_SIZE = 20;
constexpr short MAX_USER = 9000;
constexpr short MAX_ROOM = 3000;
constexpr short MAX_USER_PER_ROOM = 3;
constexpr short MAX_MONSTER_PER_ROOM = 10;
constexpr short W_WIDTH = 400;
constexpr short W_HEIGHT = 400;
constexpr short MELEEATTACK_RANGE = 5;
constexpr short LONGRANGETTACK_RANGE = 150;
constexpr short STAGE_SIZE = 1200;
constexpr short AREA_SIZE = 200;
constexpr short OBJECT_ARRAY_SIZE = 24;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_SIGNUP = 1;
constexpr char CS_SIGNIN = 2;
constexpr char CS_MOVE = 3;
constexpr char CS_ROTATE = 4;
constexpr char CS_ATTACK = 5;
constexpr char CS_INTERACTION = 6;
constexpr char CS_CHANGEWEAPON = 7;

constexpr char SC_LOGIN_INFO = 8;
constexpr char SC_ADD_PLAYER = 9;
constexpr char SC_REMOVE_PLAYER = 10;
constexpr char SC_MOVE_PLAYER = 11;
constexpr char SC_ROTATE_PLAYER = 12;
constexpr char SC_SUMMON_MONSTER = 13;
constexpr char SC_MOVE_MONSTER = 14;
constexpr char SC_OPEN_DOOR = 15;
constexpr char SC_LOGIN_COMPLETE = 16;
constexpr char SC_GAME_CLEAR = 17;
constexpr char SC_INTERACTION = 18;

#include "stdafx.h"

#define _STRESS_TEST

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	//char	name[NAME_SIZE];
};
constexpr short CS_LOGIN_PACKET_SIZE = sizeof(CS_LOGIN_PACKET);

struct CS_SIGN_PACKET {
	unsigned char size;
	char	type;
	wchar_t id[ID_SIZE];
	wchar_t password[PASSWORD_SIZE];
};
constexpr short CS_SIGN_PACKET_SIZE = sizeof(CS_SIGN_PACKET);

//struct CS_SIGNIN_PACKET {
//	unsigned char size;
//	char	type;
//	char id[NAME_SIZE];
//	char password[NAME_SIZE];
//};
//constexpr short CS_SIGNIN_PACKET_SIZE = sizeof(CS_SIGNIN_PACKET);

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	DWORD	direction = 0;
	short	id;
	XMFLOAT3 pos;
	XMFLOAT3 vel;

#ifdef _STRESS_TEST
	unsigned	move_time;
#endif
};
constexpr short CS_MOVE_PACKET_SIZE = sizeof(CS_MOVE_PACKET);

struct CS_ROTATE_PACKET {
	unsigned char size;
	char	type;
	short	id;
	float cxDelta = 0.f;
	float cyDelta = 0.f;
	float czDelta = 0.f;
};
constexpr short CS_ROTATE_PACKET_SIZE = sizeof(CS_ROTATE_PACKET);

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short CS_ATTACK_PACKET_SIZE = sizeof(CS_ATTACK_PACKET);

struct CS_INTERACTION_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short CS_INTERACTION_PACKET_SIZE = sizeof(CS_INTERACTION_PACKET);

struct CS_CHANGEWEAPON_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short cur_weaponType;
};
constexpr short CS_CHANGEWEAPON_PACKET_SIZE = sizeof(CS_CHANGEWEAPON_PACKET);

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short SC_LOGIN_INFO_PACKET_SIZE = sizeof(SC_LOGIN_INFO_PACKET);

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short cur_weaponType;
	XMFLOAT3 Pos, Look, Right, Up;
};
constexpr short SC_ADD_PLAYER_PACKET_SIZE = sizeof(SC_ADD_PLAYER_PACKET);

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
};
constexpr short SC_REMOVE_PLAYER_PACKET_SIZE = sizeof(SC_REMOVE_PLAYER_PACKET);

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	float	HP;
	XMFLOAT3  Pos;
	DWORD direction;
	XMFLOAT3 vel;
#ifdef _STRESS_TEST
	unsigned	move_time;
#endif
};
constexpr short SC_MOVE_PLAYER_PACKET_SIZE = sizeof(SC_MOVE_PLAYER_PACKET);

struct SC_ROTATE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 Look, Right;
};
constexpr short SC_ROTATE_PLAYER_PACKET_SIZE = sizeof(SC_ROTATE_PLAYER_PACKET);

struct SC_SUMMON_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
#ifdef _STRESS_TEST
	short room_num; // stress test를 위해 사용하는 임시 변수(추후 삭제 예정)
#endif
	short monster_type;
	XMFLOAT3 Pos;
};
constexpr short SC_SUMMON_MONSTER_PACKET_SIZE = sizeof(SC_SUMMON_MONSTER_PACKET);

struct SC_MOVE_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	target_id;
#ifdef _STRESS_TEST
	short room_num; // stress test를 위해 사용하는 임시 변수(추후 삭제 예정)
#endif
	XMFLOAT3 Pos;
	short HP;
	bool is_alive;
	XMFLOAT3 BulletPos;
	unsigned short animation_track; // 애니메이션 타입
};
constexpr short SC_MOVE_MONSTER_PACKET_SIZE = sizeof(SC_MOVE_MONSTER_PACKET);

struct SC_OPEN_DOOR_PACKET {
	unsigned char size;
	char	type;
	short	door_num;

};
constexpr short SC_OPEN_DOOR_PACKET_SIZE = sizeof(SC_OPEN_DOOR_PACKET);

struct SC_LOGIN_COMPLETE_PACKET {
	unsigned char size;
	char	type;
	bool	success;
};
constexpr short SC_LOGIN_COMPLETE_PACKET_SIZE = sizeof(SC_LOGIN_COMPLETE_PACKET);

struct SC_GAME_CLEAR_PACKET {
	unsigned char size;
	char	type;
	short	id;
};
constexpr short SC_GAME_CLEAR_PACKET_SIZE = sizeof(SC_GAME_CLEAR_PACKET);

struct SC_INTERACTION_PACKET {
	unsigned char size;
	char	type;
	short	stage_id;
	short	obj_id;
};
constexpr short SC_INTERACTION_PACKET_SIZE = sizeof(SC_INTERACTION_PACKET);
#pragma pack (pop)