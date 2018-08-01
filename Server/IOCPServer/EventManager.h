#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>
#include <Windows.h>
#include <process.h>
#include <time.h>
#include <random>
#include <math.h>
#include <algorithm>

#include "Deflag.h"
#include "PacketManager.h"

using namespace std;

class EventManager {
public:
	EventManager();
	~EventManager();

	bool init();
	
	bool getFlag();
	void setFlag();

	EventPacket* getEvent();
	bool setEvent();
	bool empty()
	{
		return eventQueue_.empty();
	}
	void tic(float time)
	{
		for (auto iter = eventQueue_.begin(); iter != eventQueue_.end(); iter++) 
			iter->time -= time * 0.001;
	}
	bool inEvent(SOCKET sock, float position[3]);
private:
	// EventInfo, Effect
	vector<EventInfo>eventQueue_;
	bool stopFlag_;
	HANDLE hThread_;
	vector<SOCKET> affectedUser;
};