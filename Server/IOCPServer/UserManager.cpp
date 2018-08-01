#include "UserManager.h"

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

bool UserManager::exitUser(SOCKET sock)
{
	// 해당 유저 해제
	return true;
}

bool UserManager::setJob(SOCKET sock)
{
	return false;
}

void * UserManager::getJob(SOCKET sock)
{
	
}

bool UserManager::enterUser(SOCKET sock)
{
	Client new_client;
	new_client.sock = sock;
	memset(&new_client.sock_addr, 0, sizeof(SOCKADDR_IN));
	//할당
	return true;
}
