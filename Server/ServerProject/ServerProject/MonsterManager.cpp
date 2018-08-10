#include "stdafx.h"
#include "MonsterManager.h"

MonsterManager::MonsterManager()
{
}

MonsterManager::MonsterManager(UserManager * userManager)
{
	_userManager = userManager;
	tic = clock();
}

MonsterManager::~MonsterManager()
{
	_userManager = NULL;
	monsterQueue_.clear();
}


char* MonsterManager::getMonsterInfo()
{
	return (char*)&_monster;
}

void MonsterManager::injectEmo(int emotion[4])
{
	lock_guard<mutex>lock(_lock);
	int type = 0;
	for (int i = 0; i < 4; i++) {
		_monster.emotion[i] += emotion[i];
		if (emotion[i] != 0)
			type = i;
	}
	setEmostate(type);
}

int MonsterManager::getEmotion()
{
	return _emotion;
}

int MonsterManager::getDmg()
{
	return _monster.dmg;
}

void MonsterManager::setJob(char* data)
{
	lock_guard<mutex>lock(_lock);
	if(data)
		monsterQueue_.push(data);
}

char* MonsterManager::getJob()
{
	char* buf = NULL;
	if (!monsterQueue_.empty())
		monsterQueue_.try_pop(buf);
	
	return buf;
}

void MonsterManager::upDate()
{
	clock_t tok = clock() - tic;
	switch (_emotion)
	{
	case 0:case 1:case 2:break;
	// 공격성
	case 3:case 4:
	{
		// 가까이 있으면 공격 / 가까운 유저 확인->이동
		int userIndex = 0;
		float min = INT_MAX;
		float* pos = NULL;
		for (int i = 0; i < _userManager->size(); i++) {
			pos = _userManager->getUserPos(i);
			float distance = sqrt(pow(pos[0] - _monster.position[0], 2)
				+ pow(pos[1] - _monster.position[1], 2)
				+ pow(pos[2] - _monster.position[2], 2));
			if (distance < min && _userManager->alive(i)) {
				min = distance;
				userIndex = i;
			}
		}
		// 거리내 있으면 공격
		if (min <= 1.5 && tok >= ATK_SPEED) {
			Monster_ATK* atk = new Monster_ATK;
			// 대상에서 x 범위 내에 있는 유저 검색
			atk = new Monster_ATK;
			atk->type = MONSTER_PACKET_ATK;
			atk->target = userIndex;
			atk->dmg = _monster.dmg;
			setJob((char*)atk);
			/*
			pos = _userManager->getUserPos(userIndex);
			for (int uindex = 0; uindex < _userManager->size(); uindex++) {
			float* tartgets = _userManager->getUserPos(uindex);
			float distance = sqrt(pow(pos[0] - tartgets[0], 2)
			+ pow(pos[1] - tartgets[1], 2)
			+ pow(pos[2] - tartgets[2], 2));
			if (distance <= 0.5) {
			atk = new Monster_ATK;
			atk->type = MONSTER_PACKET_ATK;
			atk->target = uindex;
			atk->dmg = _monster.dmg;
			setJob((char*)atk);
			}
			*/

			// 공격 주기를 위한 clock tic 재설정
			tic = clock();
		}
		// 없으면 이동
		else if (min > 1.5) {
			// 이동/정보 set
			setDirection(userIndex, min);
			setJob(getMonsterInfo());
		}
	}
	default:
		break;
	}
	// emotion 갱신
	if ((clock() - emoTic) > 1000) {	
		for (int i = 0; i < 4; i++)
			_monster.emotion[i] -= 1;
		emoTic = clock();
	}
}

void MonsterManager::start()
{
	flag = true;
}

bool MonsterManager::getStart()
{
	return flag;
}

void MonsterManager::setEmostate(int lastEmo)
{
	int max = 0;
	for (int i = 0; i < 4; i++) {
		if (_monster.emotion[i] > max) {
			max = _monster.emotion[i];
			_emotion = i + 1;
		}
		else if (_monster.emotion[i] == max) {
			_emotion = lastEmo + 1;
		}
	}
	if (max == 0)
		_emotion = 0;
}

void MonsterManager::setDirection(int userIndex, float distance)
{
	cout << "setDirection>>" << userIndex << endl;
	float* pos = _userManager->getUserPos(userIndex);
	for (int i = 0; i < 3; i++) {
		_monster.rotation[i] = (pos[i] - _monster.position[i]) / distance;
		_monster.position[i] += _monster.rotation[i] * _monster.speed * 0.03;
	}
}
