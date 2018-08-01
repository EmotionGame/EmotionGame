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

	EventPacket* getEvent()
	{
		EventPacket* event = NULL;
		vector<EventInfo>::iterator iter;
		for (iter = eventQueue_.begin(); iter != eventQueue_.end();) {
			if (iter->time <= 0) {
				event = iter->eventPacket;
				iter = eventQueue_.erase(iter);
				break;
			}
			else { iter++; }
		}
		return event;
	}
	void setEvent(EventPacket event)
	{
		//eventQueue_.push_back(event);
	}
	bool empty()
	{
		return eventQueue_.empty();
	}
	void tic(float time)
	{
		for (auto iter = eventQueue_.begin(); iter != eventQueue_.end(); iter++) 
			iter->time -= time * 0.001;
	}
private:
	// EventInfo, Effect
	vector<EventInfo>eventQueue_;
	bool stopFlag_;
	HANDLE hThread_;
};