// protocol.h

constexpr short PORT_NUM = 3500;
constexpr short BUF_SIZE = 512;
constexpr short IDPW_SIZE = 11;
constexpr short MAX_USER = 9000;
constexpr short MAX_ROOM = 3000;
constexpr short MAX_USER_PER_ROOM = 3;
constexpr short MAX_MONSTER_PER_ROOM = 10;
constexpr short W_WIDTH = 400;
constexpr short W_HEIGHT = 400;
constexpr short STAGE_SIZE = 1200;
constexpr short AREA_SIZE = 200;
constexpr short OBJECT_ARRAY_SIZE = 24;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_SIGNUP = 1;
constexpr char CS_SIGNIN = 2;
constexpr char CS_HEARTBEAT = 3;
constexpr char CS_ROTATE = 4;
constexpr char CS_ATTACK = 5;
constexpr char CS_INTERACTION = 6;
constexpr char CS_CHANGEWEAPON = 7;

constexpr char SC_GAME_START = 8;
constexpr char SC_ADD_PLAYER = 9;
constexpr char SC_REMOVE_PLAYER = 10;
constexpr char SC_UPDATE_PLAYER = 11;
constexpr char SC_ROTATE_PLAYER = 12;
constexpr char SC_ATTACK = 13;
constexpr char SC_CHANGEWEAPON = 14;
constexpr char SC_SUMMON_MONSTER = 15;
constexpr char SC_MOVE_MONSTER = 16;
constexpr char SC_OPEN_DOOR = 17;
constexpr char SC_SIGNUP = 18;
constexpr char SC_SIGNIN = 19;
constexpr char SC_GAME_CLEAR = 20;
constexpr char SC_INTERACTION = 21;
constexpr char SC_MONSTER_DAMAGED = 22;

#include "stdafx.h"

#define _STRESS_TEST

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
};
constexpr short CS_LOGIN_PACKET_SIZE = sizeof(CS_LOGIN_PACKET);

struct CS_SIGN_PACKET {
	unsigned char size;
	char	type;
	wchar_t id[IDPW_SIZE];
	wchar_t password[IDPW_SIZE];
};
constexpr short CS_SIGN_PACKET_SIZE = sizeof(CS_SIGN_PACKET);

struct CS_HEARTBEAT_PACKET {
	unsigned char size;
	char	type;
	short	direction = 0;
	XMFLOAT3 pos;
	XMFLOAT3 vel;
#ifdef _STRESS_TEST
	unsigned	move_time;
#endif
};
constexpr short CS_MOVE_PACKET_SIZE = sizeof(CS_HEARTBEAT_PACKET);

struct CS_ROTATE_PACKET {
	unsigned char size;
	char	type;
	float cxDelta = 0.f;
	float cyDelta = 0.f;
	float czDelta = 0.f;
};
constexpr short CS_ROTATE_PACKET_SIZE = sizeof(CS_ROTATE_PACKET);

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
};
constexpr short CS_ATTACK_PACKET_SIZE = sizeof(CS_ATTACK_PACKET);

struct CS_INTERACTION_PACKET {
	unsigned char size;
	char	type;
};
constexpr short CS_INTERACTION_PACKET_SIZE = sizeof(CS_INTERACTION_PACKET);

struct CS_CHANGEWEAPON_PACKET {
	unsigned char size;
	char	type;
};
constexpr short CS_CHANGEWEAPON_PACKET_SIZE = sizeof(CS_CHANGEWEAPON_PACKET);

struct SC_GAME_START_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short SC_GAME_START_PACKET_SIZE = sizeof(SC_GAME_START_PACKET);

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 Pos;
};
constexpr short SC_ADD_PLAYER_PACKET_SIZE = sizeof(SC_ADD_PLAYER_PACKET);

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
};
constexpr short SC_REMOVE_PLAYER_PACKET_SIZE = sizeof(SC_REMOVE_PLAYER_PACKET);

struct SC_UPDATE_PLAYER_PACKET {
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
constexpr short SC_UPDATE_PLAYER_PACKET_SIZE = sizeof(SC_UPDATE_PLAYER_PACKET);

struct SC_ROTATE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 Look, Right;
};
constexpr short SC_ROTATE_PLAYER_PACKET_SIZE = sizeof(SC_ROTATE_PLAYER_PACKET);


struct SC_ATTACK_PACKET {
	unsigned char size;
	char	type;
	short	id;
};
constexpr short SC_ATTACK_PACKET_SIZE = sizeof(SC_ATTACK_PACKET);

struct SC_CHANGEWEAPON_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	cur_weaponType;
};
constexpr short SC_CHANGEWEAPON_PACKET_SIZE = sizeof(SC_CHANGEWEAPON_PACKET);

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
	short animation_track; // 애니메이션 타입
};
constexpr short SC_MOVE_MONSTER_PACKET_SIZE = sizeof(SC_MOVE_MONSTER_PACKET);

struct SC_OPEN_DOOR_PACKET {
	unsigned char size;
	char	type;
	short	door_num;

};
constexpr short SC_OPEN_DOOR_PACKET_SIZE = sizeof(SC_OPEN_DOOR_PACKET);

struct SC_SIGN_PACKET {
	unsigned char size;
	char	type;
	bool	success;
};
constexpr short SC_SIGN_PACKET_SIZE = sizeof(SC_SIGN_PACKET);

struct SC_GAME_CLEAR_PACKET {
	unsigned char size;
	char	type;
};
constexpr short SC_GAME_CLEAR_PACKET_SIZE = sizeof(SC_GAME_CLEAR_PACKET);

struct SC_INTERACTION_PACKET {
	unsigned char size;
	char	type;
	short	stage_id;
	short	obj_id;
};
constexpr short SC_INTERACTION_PACKET_SIZE = sizeof(SC_INTERACTION_PACKET);

struct SC_MONSTER_DAMAGED_PACKET {
	unsigned char size;
	char	type;
	short	player_id;
	short	monster_id;
	short	remain_HP;
};
#pragma pack (pop)

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}