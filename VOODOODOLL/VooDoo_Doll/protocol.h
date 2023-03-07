// protocol.h

constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 512;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 10000;
constexpr int MAX_USER_PER_ROOM = 4;
constexpr int MAX_MONSTER_PER_ROOM = 10;

constexpr int W_WIDTH = 400;
constexpr int W_HEIGHT = 400;

constexpr int VIEW_RANGE = 4;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;
constexpr char SC_SUMMON_MONSTER = 6;
constexpr char SC_MOVE_MONSTER = 7;
#include "stdafx.h"


#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};
constexpr short CS_LOGIN_PACKET_SIZE = sizeof(CS_LOGIN_PACKET);

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	DWORD	direction = 0;
	short	id;
	float cxDelta, cyDelta, czDelta;
	XMFLOAT3 pos;
	//unsigned move_time;
};

constexpr short CS_MOVE_PACKET_SIZE = sizeof(CS_MOVE_PACKET);

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
	short	character_num;
	XMFLOAT3 Look, Up, Right, Pos;
	DWORD direction;
	//unsigned int move_time;
};
constexpr short SC_MOVE_PLAYER_PACKET_SIZE = sizeof(SC_MOVE_PLAYER_PACKET);


struct SC_SUMMON_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short monster_type;
	XMFLOAT3 Pos;
	//XMFLOAT3 Look, Up, Right;
};
constexpr short SC_SUMMON_MONSTER_PACKET_SIZE = sizeof(SC_SUMMON_MONSTER_PACKET);

struct SC_MOVE_MONSTER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	XMFLOAT3 Pos;
	short HP;
	unsigned short attack; // 공격 타입. 0이면 안하는 것
	//XMFLOAT3 Look, Up, Right;
	//DWORD direction;
};
constexpr short SC_MOVE_MONSTER_PACKET_SIZE = sizeof(SC_MOVE_MONSTER_PACKET);
#pragma pack (pop)