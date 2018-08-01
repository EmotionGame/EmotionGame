#include "stdafx.h"
#include "EventManager.h"
// Thread

EventManager::EventManager()
{
	stopFlag_ = false;
}

EventManager::~EventManager()
{
}

bool EventManager::init()
{
	// insert random event into event queue
	// random -> 메르겐트위스터? or fixed event
	srand(time(NULL));

	EventInfo eventInfo;
	for (int i = 1; i <= 10; i++) {
		EventPacket* event = new EventPacket;
		// make Event & struct of Information
		event->type = EVENT_PACKET;
		event->id = i;
		int randomValue = rand() % 171;
		int randomTime = rand() % 22;
		event->position[0] = (randomValue % 23) % 11;
		if (rand() % 2)
			event->position[0] *= -1;
		event->position[1] = 0;
		event->position[2] = (randomValue % 31) % 11;
		if (rand() % 2)
			event->position[2] *= -1;
		event->state = true;
		eventInfo.time = randomValue % 30;
		eventInfo.eventPacket = event;
		eventQueue_.push_back(eventInfo);
	}
	cout << "Event init End and size : " << eventQueue_.size() << endl;
	return true;
}
bool EventManager::getFlag()
{
	return stopFlag_;
}

void EventManager::setFlag()
{
	stopFlag_ = true;
}

