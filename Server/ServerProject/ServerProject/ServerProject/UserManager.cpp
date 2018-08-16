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
		info.acceleration[i] = 0.0;
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
void UserManager::start()
{
	lock_guard<mutex>lock(g_mutex);
	for (int i = 0; i < users.size(); i++) 
		users[i].second->userInfo.type = 31;
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
		users[index].second->userInfo.acceleration[i] = update->acceleration[i];
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
		users[index].second->userInfo.acceleration[i] = action->acceleration[i];
	}
	cout << "SetUserPos>>" << action->position[0] << action->position[1] << action->position[2] << endl;
	return true;
}
float* UserManager::getUserPos(int index)
{
	return users[index].second->userInfo.position;
}
void UserManager::setUserEmotion(int index, int emotion[4])
{
	lock_guard<mutex>lock(c_mutex[index]);
	// 현재의 가장 높은 감정 저장/변경되었다면 유저 세팅 변경
	int current_emotion = 0;
	for (int i = 0; i < 4; i++) {
		if (users[index].second->userInfo.emotion[i] > current_emotion)
			current_emotion = users[index].second->userInfo.emotion[i];
	}
	for (int i = 0; i < 4; i++) {
		if (users[index].second->userInfo.emotion[i] + emotion[i] <= 100 && users[index].second->userInfo.emotion[i] + emotion[i] >= 0) 
			users[index].second->userInfo.emotion[i] += emotion[i];
	}
	effect(index, current_emotion);
}
void UserManager::setUserHp(int index, int dmg)
{
	lock_guard<mutex>lock(c_mutex[index]);
	users[index].second->userInfo.hp -= dmg;
	if (users[index].second->userInfo.hp <= 0)
		users[index].second->userInfo.hp = 0;
}
void UserManager::update()
{
	lock_guard<mutex>lock(g_mutex);
	// 1초마다 감정으로 인한 효과 적용 후 감정 감소
	int emotionUpdate[4] = { -1,-1,-1,-1 };
	for (int i = 0; i < users.size(); i++) {
		int current_emotion = 0;
		for (int ei = 0; ei < 4; ei++) {
			if (users[i].second->userInfo.emotion[ei] > current_emotion)
				current_emotion = users[i].second->userInfo.emotion[ei];
		}
		effect(i, current_emotion);
		setUserEmotion(i, emotionUpdate);
	}
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

void UserManager::effect(int index, int current_emotion)
{
	//lock_guard<mutex>lock(c_mutex[index]);
	// 무감각,기쁨,분노,공포(속도),놀람(크기)
	switch (current_emotion)
	{
	case 0:
	case 1:break;
	case 2:users[index].second->userInfo.hp /= 2;
		break;
	case 3:users[index].second->userInfo.speed = 10.0;
		break;
	case 4:
		users[index].second->userInfo.scale[0] = 1.0;
		users[index].second->userInfo.scale[1] = 1.0;
		users[index].second->userInfo.scale[2] = 1.0;
		break;
	default:
		break;
	}
	
	int new_emotion = 0;
	for (int i = 0; i < 4; i++) {
		if (users[index].second->userInfo.emotion[i] > current_emotion) 
			new_emotion = users[index].second->userInfo.emotion[i];
	}
	
	switch (new_emotion)
	{
	case 0: break;
	case 1: users[index].second->userInfo.hp += 1;
		break;
		// 분노
	case 2:users[index].second->userInfo.hp *= 2;
		break;
		// 공포 - 속도
	case 3:users[index].second->userInfo.speed = 11.0;
		break;
		// 놀람 - 크기
	case 4:
		users[index].second->userInfo.scale[0] = 0.5;
		users[index].second->userInfo.scale[1] = 0.5;
		users[index].second->userInfo.scale[2] = 0.5;
		break;
	}
}

bool UserManager::alive(int index)
{
	if (users[index].second->userInfo.hp <= 0)
		return false;
	return true;
}

int UserManager::size()
{
	return users.size();
}
