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
#define USER_EVENT_PACKET 22

#define USER_PACKET   30
#define USER_UPDATE_PACKET 31

#define MONSTER_PACKET 50
#define USER_MONSTER_PACKET 51
#define MONSTER_PACKET_ATK    52

#define VIEW_PLAYER  60
#define VIEW_MONSTER 61
#define VIEW_OBJECT  62

#define OBJECT_PACKET 70

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
	float acceleration[3] = { 0.0f, 0.0f, 0.0f };
}User, *pUser;

typedef struct {
	int type;
	int id;
	float position[3];
	float rotation[3];
	float acceleration[3] = { 0.0f, 0.0f, 0.0f };
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
	int dmg = 20;
}Monster;

typedef struct {
	int type;
	int target;
	int dmg;
}Monster_ATK;

typedef struct {
	int type;
	int eventId;
	int userId;
	float pos[3];
}UserEventPacket;

typedef struct {
	int type = 51;
	int playerId = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	bool collision = false;
}UserMonsterPacket;


typedef struct {
	int type = 60;
	int player1Id = 0; // 감정을 전달하는 플레이어
	int player2Id = 0; // 감정을 전달 당하는 플레이어
	int emotion; // 전달된 감정 수치
}Player2Player;

typedef struct {
	int type = 61;
	int emotion;
}Player2Monster;

typedef struct {
	int type = 62;
	int objectId = 0; // 감정이 상승되는 오브젝트
	int emotion; // 상승된 감정 수치
}Player2Object;