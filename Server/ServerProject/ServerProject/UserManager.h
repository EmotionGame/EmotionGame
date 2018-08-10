#pragma once
#include <iostream>
#include <WinSock2.h>
// concurrent Queue
#include <vector>
#include <utility>
#include <tbb\concurrent_queue.h>
#include <mutex>
#include <deque>

#include "Deflag.h"

using namespace std;

typedef struct {
	User userInfo;
	tbb::concurrent_queue<char*> jobBuf;
}Client, *pClient;

class UserManager {
public:
	UserManager();
	~UserManager();

	int enterUser(SOCKET sock);
	bool exitUser(SOCKET sock, int index);

	bool setUserInfo(int index, char* data);
	char* getUserInfo(int index);

	bool setUserPos(int index, char* data);
	float* getUserPos(int index);

	void setUserEmotion(int index, int emotion[4]);
	void setUserHp(int index, int dmg);

	bool setJob(int index, char* data);
	char* getJob(int index);

	bool alive(int index);

	int size();
private:
	int userCount;
	std::vector<pair<SOCKET, Client*>>users;
	mutex g_mutex;
	mutex c_mutex[USERSIZE];
};