#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <mutex>
#include <random>

#include "Deflag.h"

#define DISTANCE 3

using namespace std;

class EventManager
{
public:
	EventManager();
	~EventManager();

	bool init();
	EventPacket* randomEvent(int id);
	void pushEvent(EventPacket* event, float time);
	int size();

	float getEventTime(int i);
	char* getEvent(int i);

	int* getEmotion(int i);

	bool validate(int i, float pos[3]);

	// yÁÂÇ¥ °»½Å
	void setEvent(EventPacket* event);

	void updateEvent(int i, clock_t time);
	bool userInEvent(int i, float pos[3]);

private:
	int randomValue(clock_t t);
	vector<EventInfo>eventQueue_;
	mutex g_mutex;
};