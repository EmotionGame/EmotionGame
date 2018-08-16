#include "stdafx.h"
#include "Object.h"

Object::Object()
{
	for (int i = 0; i < 4; i++) {
		ObjectPacket object;
		object.id = i;
		object.position[0] = randomValue();
		object.position[2] = randomValue();
		objects.push_back(make_pair(i/2+1, object));
	}
}

Object::~Object()
{
	objects.clear();
}

void Object::start()
{
	for (int i = 0; i < objects.size(); i++)
		objects[i].second.state = true;
}

bool Object::validation(int i, float pos[3], bool collision)
{
	lock_guard<mutex>lock(_lock);
	float distance = sqrt(pow(objects[i].second.position[0] - pos[0], 2) + pow(objects[i].second.position[2] - pos[2], 2));
	// // // // // // // // // // // // // cout << "Object Validate>>Distance>>" << distance << endl;
	if (distance <= 256) {
		return collision;
	}
	return collision;
}

char * Object::getObject(int index)
{
	return reinterpret_cast<char*>(&objects[index].second);
}

void Object::setEmotion(int index, int emotion[4])
{
	lock_guard<mutex>lock(_lock);
	int current_emotion = 0;
	for (int i = 0; i < 4; i++) {
		if (objects[index].second.emotion[i] > current_emotion)
			current_emotion = i + 1;
	}

	for (int i = 0; i < 4; i++) {
		if (objects[index].second.emotion[i] + emotion[i] >= 100)
			objects[index].second.emotion[i] = 100;
		else if (objects[index].second.emotion[i] + emotion[i] <= 0)
			objects[index].second.emotion[i] = 0;
		else
			objects[index].second.emotion[i] += emotion[i];
	}
	int new_emotion = 0;
	for (int i = 0; i < 4; i++) {
		if (objects[index].second.emotion[i] > new_emotion)
			new_emotion = i+1;
	}
	if (index == 1 || index == 2) {
		if (current_emotion != new_emotion) {
			if (new_emotion == 4)
				setScale(index, true);
			else
				setScale(index, false);
		}
	}
}

void Object::damaged(int index, int dmg)
{
	lock_guard<mutex>lock(_lock);
	objects[index].second.hp -= dmg;

	if (objects[index].second.hp <= 0)
		objects[index].second.state = false;
}

void Object::setScale(int index, bool flag)
{
	if (flag) {
		objects[index].second.scale[0] = 1.5;
		objects[index].second.scale[1] = 1.5;
		objects[index].second.scale[2] = 1.5;
	}
	else {
		objects[index].second.scale[0] = 1.0;
		objects[index].second.scale[1] = 1.0;
		objects[index].second.scale[2] = 1.0;
	}
	changed[index] = true;
}

int Object::effect(int index)
{
	if (index == 1) {
		return 1;
	}
	else if (index == 2 && cooltime>=3000) {
		cooltime = clock();
		return 2;
	}
	return 0;
}

bool Object::getScaleChanged(int i)
{
	return changed[i];
}
void Object::updatedScale(int i)
{
	changed[i] = false;
}
void Object::update()
{
	lock_guard<mutex>lock(_lock);
	int emotionUpdate[4] = { -1,-1,-1,-1 };

	for (int i = 0; i < objects.size(); i++) 
		setEmotion(i, emotionUpdate);
}

float Object::randomValue()
{
	int random;
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(0, 60);
	random = range(rnd);
	srand(clock());
	if (rand() % 2 == 0)
		random *= -1;
	return 128+random;
}
