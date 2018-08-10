#include "stdafx.h"
#include "UserManager.h"

UserManager::UserManager()
{
	userCount = 0;
}
UserManager::~UserManager()
{

}

int UserManager::enterUser(SOCKET sock)
{
	lock_guard<mutex>lock(g_mutex);
	int index = -1;
	Client* new_client = new Client();

	User info;
	info.type = USER_PACKET;
	info.id = sock;
	info.hp = 100;
	info.speed = 10;
	for (int i = 0; i<4; i++)
		info.emotion[i] = 0;
	info.position[0] = 1.0;
	info.position[1] = 0.0;
	info.position[2] = 1.0;
	for (int i = 0; i < 3; i++) {
		info.rotation[i] = 0.0;
		info.scale[i] = 0.0; 
	}
	new_client->userInfo = info;
	new_client->jobBuf.clear();

	users.push_back(make_pair(sock, new_client));
	return users.size() - 1;
}
bool UserManager::exitUser(SOCKET sock, int index)
{
	lock_guard<mutex>lock(c_mutex[index]);
	users[index].first = -1;
	free(users[index].second);
	return true;
}
bool UserManager::setUserInfo(int index, char* data)
{
	lock_guard<mutex>lock(c_mutex[index]);
	User* update = reinterpret_cast<User*>(data);
	users[index].second->userInfo.hp = update->hp;
	users[index].second->userInfo.speed = update->speed;

	for (int i = 0; i < 3; i++) {
		users[index].second->userInfo.position[i] = update->position[i];
		users[index].second->userInfo.rotation[i] = update->rotation[i];
	}
	for (int i = 0; i<4; i++)
		users[index].second->userInfo.emotion[i] = update->emotion[i];
	return true;
}
char* UserManager::getUserInfo(int index)
{
	lock_guard<mutex>lock(c_mutex[index]);
	return reinterpret_cast<char*>(&users[index].second->userInfo);
}
bool UserManager::setUserPos(int index, char* data)
{
	lock_guard<mutex>lock(c_mutex[index]);
	ActionPacket* action = reinterpret_cast<ActionPacket*>(data);
	for (int i = 0; i < 3; i++) {
		users[index].second->userInfo.position[i] = action->position[i];
		users[index].second->userInfo.rotation[i] = action->rotation[i];
	}
	return true;
}
float* UserManager::getUserPos(int index)
{
	return users[index].second->userInfo.position;
}
void UserManager::setUserEmotion(int index, int emotion[4])
{
	lock_guard<mutex>lock(c_mutex[index]);
	for (int i = 0; i < 4; i++)
		users[index].second->userInfo.emotion[i] += emotion[i];
}
void UserManager::setUserHp(int index, int dmg)
{
	lock_guard<mutex>lock(c_mutex[index]);
	users[index].second->userInfo.hp -= dmg;
	if (users[index].second->userInfo.hp <= 0)
		users[index].second->userInfo.hp = 0;
}
bool UserManager::setJob(int index, char* data)
{
	lock_guard<mutex>lock(c_mutex[index]);
	if (data == NULL)
		return false;
	users[index].second->jobBuf.push(data);
	return true;
}
char* UserManager::getJob(int index)
{
	char* buf = NULL;
	users[index].second->jobBuf.try_pop(buf);
	return buf;
}

bool UserManager::alive(int index)
{
	if(users[index].second->userInfo.hp <= 0)
		return false;
	return true;
}

int UserManager::size()
{
	return users.size();
}
