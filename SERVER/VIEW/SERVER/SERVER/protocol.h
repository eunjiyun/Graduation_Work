// protocol.h

constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 512;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 10000;
constexpr int MAX_ROOM = 2500;
constexpr int MAX_USER_PER_ROOM = 4;
constexpr int MAX_MONSTER_PER_ROOM = 10;

constexpr int W_WIDTH = 400;
constexpr int W_HEIGHT = 400;

constexpr int VIEW_RANGE = 4;
constexpr int STAGE_SIZE = 600;


// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;
constexpr char CS_COLLECT = 3;
constexpr char CS_CHANGEWEAPON = 4;

constexpr char SC_LOGIN_INFO = 5;
constexpr char SC_ADD_PLAYER = 6;
constexpr char SC_REMOVE_PLAYER = 7;
constexpr char SC_MOVE_PLAYER = 8;
constexpr char SC_SUMMON_MONSTER = 9;
constexpr char SC_MOVE_MONSTER = 10;
#include "stdafx.h"


#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	//char	name[NAME_SIZE];
};
constexpr short CS_LOGIN_PACKET_SIZE = sizeof(CS_LOGIN_PACKET);

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	DWORD	direction = 0;
	short	id;
	float cxDelta = 0.f;
	float cyDelta = 0.f;
	float czDelta = 0.f;
	XMFLOAT3 pos;
	XMFLOAT3 vel;
};
constexpr short CS_MOVE_PACKET_SIZE = sizeof(CS_MOVE_PACKET);

struct CS_ATTACK_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short CS_ATTACK_PACKET_SIZE = sizeof(CS_ATTACK_PACKET);

struct CS_COLLECT_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 pos;
};
constexpr short CS_COLLECT_PACKET_SIZE = sizeof(CS_COLLECT_PACKET);

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
	XMFLOAT3 Pos, Look, Right, Up;
	char	name[NAME_SIZE];
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
	XMFLOAT3 Look, Up, Right, Pos;
	DWORD direction;
	XMFLOAT3 BulletPos;
	XMFLOAT3 vel;
};
constexpr short SC_MOVE_PLAYER_PACKET_SIZE = sizeof(SC_MOVE_PLAYER_PACKET);


struct SC_SUMMON_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short monster_type;
	XMFLOAT3 Pos;
};
constexpr short SC_SUMMON_MONSTER_PACKET_SIZE = sizeof(SC_SUMMON_MONSTER_PACKET);

struct SC_MOVE_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 Pos;
	short HP;
	bool is_alive;
	short Chasing_PlayerID;
	XMFLOAT3 BulletPos;
	unsigned short animation_track; // 애니메이션 타입
};
constexpr short SC_MOVE_MONSTER_PACKET_SIZE = sizeof(SC_MOVE_MONSTER_PACKET);
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