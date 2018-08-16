#pragma once
#include <vector>
#include <random>
#include <time.h>
#include <mutex>
#include <iostream>
#include "Deflag.h"
using namespace std;
class Object {
public:
	Object();
	~Object();

	void start();

	bool validation(int i, float pos[3], bool collision);
	char* getObject(int index);
	void setEmotion(int index, int emotion[4]);
	void damaged(int index, int dmg);
	// � ������ ��ġ�� ���� ������ return
	void setScale(int index, bool flag);
	int effect(int index);
	bool getScaleChanged(int i);
	void updatedScale(int i);
	void update();
private:
	clock_t tic = clock(), cooltime = clock();
	float randomValue();
	// 1: ��Ÿ��, 2: ��
	vector<pair<int, ObjectPacket>>objects;
	bool changed[2];
	mutex _lock;
};