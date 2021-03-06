#pragma once
#include <mutex>
#include <vector>
#include <utility>
#include<tbb/concurrent_queue.h>
#include <math.h>
#include "Deflag.h"
#include "UserManager.h"


#define ATK_SPEED 2000

using namespace std;

class MonsterManager {
public:
	MonsterManager();
	MonsterManager(UserManager* userManager);
	~MonsterManager();

	char* getMonsterInfo();
	void injectEmo(int emotion[4]);
	// �ִ� ���� return
	int getEmotion();
	int getDmg();

	void setJob(char* data);
	char* getJob();
	void upDate();

	void start();
	bool getStart();
private:
	void setEmostate(int type);
	void setDirection(int userIndex, float distance);

	Monster _monster;
	tbb::concurrent_queue<char*> monsterQueue_;
	// 0: non 1:2: 3:4
	int _emotion = 3;
	UserManager* _userManager = NULL;
	bool flag = false;
	clock_t tic;
	clock_t emoTic = clock();
	mutex _lock;
};