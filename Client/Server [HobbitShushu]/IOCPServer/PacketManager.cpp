#include "stdafx.h"
#include "PacketManager.h"

PacketManager::PacketManager()
{
	flag = 0;
	memset(&mBuffer, 0, sizeof(mBuffer));
}

PacketManager::~PacketManager()
{
}

char * PacketManager::encode(int type, ...)
{
	va_list args;
	va_start(args, type);

	PACKET new_packet;
	new_packet.type = type;
	switch (type)
	{
	case 1:
		USER user;
		user.id = (int)va_arg(args, int);
		user.hp = 50;
		user.speed = 1.0;
		memset(user.emotion, 0, sizeof(int) * 4);
		memset(user.position, 0, sizeof(int) * 3);
		memset(user.rotation, 0, sizeof(int) * 3);
		//memcpy(new_packet.buffer, (char*)&user, sizeof(user));
		memcpy(new_packet.buffer, (char*)&user, sizeof(BUFSIZE));
		break;
	case ACTION_PACKET:
		// id, position, lookat, up
		// Struct a;
		// memcpy(a.attribute, (char*)va_arg(args, POINTER), SIZE)

		break;
	case 2:
		break;
	}
	
	va_end(args);
	return (char*)&new_packet;
	//return reinterpret_cast<char*>(&new_packet);
}
