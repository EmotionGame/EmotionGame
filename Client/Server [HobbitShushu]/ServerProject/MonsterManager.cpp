#include "stdafx.h"
#include "MonsterManager.h"

MonsterManager::MonsterManager()
{
}

MonsterManager::MonsterManager(UserManager * userManager)
{
	_userManager = userManager;
	tic = clock();
	emoTic = clock();
	targets.assign(USERSIZE, false);
}

MonsterManager::~MonsterManager()
{
	_userManager = NULL;
	targets.clear();
	monsterQueue_.clear();
	monsterQueue_.clear();
}


char* MonsterManager::getMonsterInfo()
{
	return (char*)&_monster;
}

void MonsterManager::setEmotion(int emotion[4])
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

// �ٸ� ��� �Լ� ȣ��
void MonsterManager::setMonsterState(int emo, int new_emo)
{
	switch (emo)
	{
	// 0��, ���
	case 1: 
		break;
	// �г�
	case 2: _monster.dmg /= 2;
		break;
	// ����
	case 3:
		_monster.speed = 9.0f;
		break;
	// ��� // scale ����
	case 4:
		_monster.scale[0] = 1.0;
		_monster.scale[1] = 1.0;
		_monster.scale[2] = 1.0;
		break;
	}
	switch (new_emo)
	{
	// ���
	case 1:
		break;
	// �г�
	case 2: _monster.dmg *= 2;
		break;
	// ����
	case 3:
		_monster.speed = 10.0f;
		break;
	// ��� // scale ����
	case 4:
		_monster.scale[0] = 1.5;
		_monster.scale[1] = 1.5;
		_monster.scale[2] = 1.5;
		break;
	default:
		break;
	}
	infoChanged = true;
	// // cout << "MONSTER CURRENT EMOTION>>" << new_emo << endl;
}

void MonsterManager::setJob(char* data)
{
	lock_guard<mutex>lock(_lock);
	if (data)
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
	// ���� �� 1�� ������ ���� tok ����
	if (tok >= 1000) {
		atked = false;
		// // cout << "MONSTER EMOTION>>" << _emotion << endl;
		int maxValue = 0;
		for (int i = 0; i < 4; i++) {
			if (_monster.emotion[i] > maxValue) {
				_emotion = i + 1;
				maxValue = _monster.emotion[i];
			}
		}

		switch (_emotion)
		{
		case 0:case 1:break;
		// ���ݼ�
		case 2:case 3:case 4:
		{
			// ������ ������ ���� / ����� ���� Ȯ��->�̵�
			int userIndex = 0;
			float min = MAXDISTANCE;
			float* pos = NULL;
			// ���� ����� ���� Ȯ��
			for (int i = 0; i < _userManager->size(); i++) {
				pos = _userManager->getUserPos(i);
				float distance = sqrt(pow(pos[0] - _monster.position[0], 2)
					+ pow(pos[1] - _monster.position[1], 2)
					+ pow(pos[2] - _monster.position[2], 2));
				// // cout << distance << endl;
				if (distance < min && _userManager->alive(i)) {
					min = distance;
					userIndex = i;
				}
			}

			// �Ÿ��� ������ ����
			for (int i = 0; i < targets.size(); i++) {
				if (tok >= ATK_SPEED && targets[i]) {
					Monster_ATK* atk = new Monster_ATK;
					// ��󿡼� x ���� ���� �ִ� ���� �˻�
					atk = new Monster_ATK;
					atk->type = MONSTER_PACKET_ATK;
					atk->target = i;
					atk->dmg = _monster.dmg;
					setJob((char*)atk);
					// ���� �ֱ� �� ���� �� 1�� ������ ���� tic ����
					tic = clock();
					// // cout << "----------------------------------------------ATK-------------------------------" << endl;
					atked = true;
				}
			}
			// ������ �������� �̵�
			if (atked == false) {
				// �Ÿ��� ������ �̵�
				// // cout << "ATKED>>FALSE" << endl;
				// // cout << min << endl;
				setDirection(userIndex, min);
				setJob(getMonsterInfo());
			}
			break;
		}
		default:
			break;
		}
	}
	// emotion ����
	if ((clock() - emoTic) > 1000) {
		int emotionUpdate[4] = { -1,-1,-1,-1 };
		setEmotion(emotionUpdate);
		emoTic = clock();
	}
	if (infoChanged) {
		setJob(getMonsterInfo());
		infoChanged = false;
	}
}

bool MonsterManager::validate(int i, float pos[3], bool collision)
{
	lock_guard<mutex>lock(_lock);
	float distance = sqrt(pow(_monster.position[0] - pos[0], 2) + pow(_monster.position[2] - pos[2], 2));
	// // cout << "Monster Validate>>Distance>>" << distance << endl;
	if (distance <= 256) {
		// // cout << "Monster Target>>" << i << ", Collision>>" << collision << endl;
		targets[i] = collision;
		return true;
	}
	targets[i] = collision;
	return false;
}

void MonsterManager::start()
{
	flag = true;
}

bool MonsterManager::getStart()
{
	return flag;
}

// set Emotion�� ���� lock
void MonsterManager::setEmostate(int lastEmo)
{
	int max = 0;
	int currentEmotion = _emotion;
	for (int i = 0; i < 4; i++) {
		if (_monster.emotion[i] > max) {
			max = _monster.emotion[i];
			_emotion = i + 1;
		}
	}
	if (max == 0)
		_emotion = 0;
	if (_emotion != currentEmotion)
		setMonsterState(currentEmotion, _emotion);
}

//void MonsterManager::setDirection(int userIndex, float distance)
//{
//	float* pos = _userManager->getUserPos(userIndex);
//	// // cout << "DISTANCE>>" << distance << endl;
//	
//	float rotation = static_cast<float>(atan2(_monster.position[0] - pos[0], _monster.position[2] - pos[2]));
//
//	for (int i = 0; i < 3; i++) {
//		_monster.position[i] += _monster.rotation[i] * (_monster.speed/10);
//	}
//	_monster.rotation[1] = rotation; 
//}

void MonsterManager::setDirection(int userIndex, float distance)
{
	float* pos = _userManager->getUserPos(userIndex);
	// // cout << "DISTANCE>>" << distance << endl;

	float rotation = static_cast<float>(atan2(_monster.position[0] - pos[0], _monster.position[2] - pos[2]));
	_monster.rotation[0] = (pos[0] - _monster.position[0]) / distance;
	_monster.position[0] += _monster.rotation[0] * (_monster.speed / 2);
	_monster.rotation[1] = rotation;
	_monster.position[1] += rotation * (_monster.speed / 2);
	_monster.rotation[2] = (pos[2] - _monster.position[2]) / distance;
	_monster.position[2] += _monster.rotation[2] * (_monster.speed / 2);
}