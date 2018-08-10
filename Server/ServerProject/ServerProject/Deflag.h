#pragma once

#define BUFSIZE 256
#define POOLSIZE 10
#define USERSIZE 4

#define EMOTIONSIZE 4

#define ACCEPT 1
#define READ   2
#define WRITE  3
#define POST   4
#define DISCONNECT 5

#define BROADCAST 0

#define INIT_USER     10
#define ACTION_PACKET 11
#define EVENT_PACKET  21

#define USER_PACKET   30
#define USER_UPDATE_PACKET 31
#define USER_VIEW     32

#define OBJECT_PACKET 40
#define MONSTER_PACKET 50
#define MONSTER_PACKET_ATK    51

#define DATA_NOT     0100
#define DATA_USER    0101
#define DATA_MONSTER 0110
#define DATA_EMOTION 0111

#define ERROR_SETTING  1000
#define ERROR_TRANSMIT 1001

#define QUIT   1111


typedef struct {
	int type;
	int id;
	int hp;
	float speed = 1.0f;
	int emotion[4];
	float position[3];
	float rotation[3];
	float scale[3] = { 0.0f, 0.0f, 0.0f };
}User, *pUser;

typedef struct {
	int type;
	int id;
	float position[3];
	float rotation[3];
}ActionPacket, *pActionPacket;

typedef struct {
	int type;
	int id;
	float position[3];
	bool state;
}EventPacket, pEventPacket;

typedef struct {
	EventPacket* eventPacket;
	float time;
}EventInfo, pEventInfo;

typedef struct {
	int type = OBJECT_PACKET;
	int id;
	int hp = 40;
	int emotion[4] = { 0,0,0,0 };
	float position[3];
	bool state = true;
}ObjectPacket;

typedef struct {
	int type = MONSTER_PACKET;
	float speed = 9.0f;
	int emotion[4] = { 0,0,0,0 };
	float position[3] = { 128,128,128 };
	float rotation[3] = { 0, 0, 0 };
	int dmg = 25;
}Monster;

typedef struct {
	int type;
	int target;
	int dmg;
}Monster_ATK;