#include "UserManager.h"

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

bool UserManager::exitUser(SOCKET sock)
{
	// �ش� ���� ����
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
	//�Ҵ�
	return true;
}
