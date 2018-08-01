#pragma once

// for memset
#include <string>
// va_ funtion
#include <stdarg.h>
#include <vector>
#include "Deflag.h"

using namespace std;

typedef struct {
	int type;
	char buffer[BUFSIZE];
}PACKET, *LPACKET;

typedef struct {
	int id;
	int hp;
	float speed;
	int emotion[4];
	float position[3];
	float rotation[3];
}USER, *pUSER;

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
}EventPacket, pEvnetPacket;

typedef struct {
	EventPacket* eventPacket;
	float time;
}EventInfo, pEventInfo;



// ±â»Ý ºÐ³ë °øÆ÷ ³î¶÷
/*typedef struct {
	int pleasure;
	int rage;
	int	fear;
	int	fright;
} Emotion;
*/


class PacketManager {
public:
	PacketManager();
	~PacketManager();
	
	char* encode(int type, ...);
private:
	int flag;
	char mBuffer[BUFSIZE];
};