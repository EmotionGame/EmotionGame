#include "stdafx.h"
#include "Object.h"

Object::Object()
{
	for (int i = 0; i < 2; i++) {
		ObjectPacket object;
		// id = 1, ��Ÿ��
		object.id = 1;
		object.position[0] = randomValue();
		object.position[1] = randomValue();
		objects.push_back(object);
	}
	for (int i = 0; i < 2; i++) {
		ObjectPacket object;
		// id = 2, ��
		object.id = 2;
		object.position[0] = randomValue();
		object.position[1] = randomValue();
		objects.push_back(object);
	}
}

Object::~Object()
{
	objects.clear();
}

void Object::injectEmo(int index, int emotion[4])
{
	lock_guard<mutex>lock(_lock);
	for (int i = 0; i < 4; i++)
		objects[index].emotion[i] += emotion[i];
}

void Object::damaged(int index, int dmg)
{
	lock_guard<mutex>lock(_lock);
	objects[index].hp -= dmg;

	if (objects[index].hp <= 0)
		objects[index].state = false;
}

void Object::update()
{
	lock_guard<mutex>lock(_lock);
	for (int i = 0; i < objects.size(); i++) {
		for (int ei = 0; ei < 4; ei++) {
			if (objects[i].emotion[ei] > 0)
				objects[i].emotion[ei] -= 1;
		}
	}
}

float Object::randomValue()
{
	int random;
	//< 1�ܰ�. �õ� ����
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2�ܰ�. ���� ���� ( ���� )
	uniform_int_distribution<int> range(0, 10);
	random = range(rnd);
	srand(clock());
	if (rand() % 2 == 0)
		random *= -1;
	return random;
}
