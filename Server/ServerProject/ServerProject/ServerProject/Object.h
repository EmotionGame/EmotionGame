#pragma once
#include <vector>
#include <random>
#include <time.h>
#include <mutex>

#include "Deflag.h"
using namespace std;
class Object {
public:
	Object();
	~Object();

	void start();

	void injectEmo(int index, int emotion[4]);
	void damaged(int index, int dmg);
	// 어떤 감정이 수치가 제일 높은지 return
	int getEffect(int index)
	{
		int pos = 0;
		int value = 0;
		for (int i = 0; i < 4; i++) {
			if (objects[index].second.emotion[i] > value) {
				pos = i + 1;
				value = objects[index].second.emotion[i];
			}
		}
		return pos;
	}

	void update();
private:
	clock_t tic = clock(), tok;
	float randomValue();
	// 1: 울타리, 2: 움막
	vector<pair<int, ObjectPacket>>objects;
	mutex _lock;
};