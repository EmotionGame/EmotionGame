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

pUser PacketManager::enterUser(int sock)
{
	pUser new_user = new User();
	new_user->type = USER_PACKET;
	new_user->id = sock;
	new_user->hp = 100;
	new_user->speed = 10;
	for(int i=0; i<4; i++)
		new_user->emotion[i] = 0;
	new_user->position[0] = 1.0;
	new_user->position[1] = 0.0;
	new_user->position[2] = 1.0;
	for (int i = 0; i < 3; i++)
		new_user->rotation[i] = 0.0;
	return new_user;
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
		User user;
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
