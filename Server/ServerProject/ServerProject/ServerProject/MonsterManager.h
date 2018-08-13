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
	void setEmotion(int emotion[4]);
	// 최대 감정 return
	int getEmotion();
	int getDmg();

	void setJob(char* data);
	char* getJob();
	void upDate();

	bool validate(int i, float pos[3], bool collision);

	void start();
	bool getStart();
private:
	void setEmostate(int type);
	void setDirection(int userIndex, float distance);
	void setMonsterState(int emo, int new_emo);

	Monster _monster;
	tbb::concurrent_queue<char*> monsterQueue_;
	// 0: non 1:2: 3:4
	int _emotion = 3;
	UserManager* _userManager = NULL;
	vector<bool> targets;
	bool atked = false;
	bool flag = false;
	clock_t tic;
	clock_t emoTic;
	mutex _lock;
};