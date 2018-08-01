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
	// 몬스터가 오브젝트 공격 or 패턴이 변경되는 시점
	srand(time(NULL));
	EventInfo eventInfo;
	EventPacket* event = new EventPacket;
	event->type = EVENT_PACKET;
	event->id = 100;

	event->position[0] = 0;
	event->position[1] = 0;
	event->position[2] = 0;
	event->state = true;

	eventInfo.time = 30;
	eventInfo.eventPacket = event;
	eventQueue_.push_back(eventInfo);
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

// 순회-time이 0이면 해당 event 리턴
EventPacket * EventManager::getEvent()
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

bool EventManager::setEvent()
{
	srand(time(NULL));
	EventInfo eventInfo;
	EventInfo disappear_eventInfo;
	EventPacket* event = new EventPacket;
	EventPacket* disappear_event = new EventPacket;

	event->type = EVENT_PACKET;
	event->id = rand() % 10 + 1;
	event->type = EVENT_PACKET;
	event->id = event->id;

	int randomValue = rand() % 171;
	int randomTime = rand() % 22;

	event->position[0] = (randomValue % 23) % 11;
	if (rand() % 2)
		event->position[0] *= -1;
	event->position[1] = 0;
	event->position[2] = (randomValue % 31) % 11;
	if (rand() % 2)
		event->position[2] *= -1;
	disappear_event->position[0] = event->position[0];
	disappear_event->position[1] = event->position[1];
	disappear_event->position[2] = event->position[2];

	event->state = true;
	disappear_event->state = false;

	eventInfo.time = randomValue % 30;
	eventInfo.eventPacket = event;
	eventQueue_.push_back(eventInfo);

	disappear_eventInfo.time = 5;
	disappear_eventInfo.eventPacket = disappear_event;
	eventQueue_.push_back(disappear_eventInfo);
	return true;
}

bool EventManager::inEvent(SOCKET sock, float position[3])
{
	auto iter = eventQueue_.begin();
	if (iter == eventQueue_.end())
		return false;
	if (eventQueue_.empty())
		return false;

	EventInfo event = eventQueue_.front();
	eventQueue_.erase(iter);

	float distance = sqrt(pow(event.eventPacket->position[0] - position[0], 2) +
		pow(event.eventPacket->position[2] - position[2], 2));
	
	vector<SOCKET>::iterator vecIter = find(affectedUser.begin(), affectedUser.end(), sock);
	if (vecIter == affectedUser.end() && distance > 3)
		return false;
	else if (vecIter == affectedUser.end() && distance <= 3)
		return true;
	else if (vecIter != affectedUser.end() && distance > 3)
		return false;
	else if (vecIter != affectedUser.end() && distance <= 3)
		return true;
	return false;
	/*
	for (auto iter = affectedUser.begin(); iter != affectedUser.end();iter++){
		// vector 내에 이미 socket이 존재한다면
		if (*iter == sock) {
			if (distance > 3) {
				affectedUser.erase(iter);
				return false;
			}
			else
				return true;
		}
		// 존재하지 않으면
		else {
			if (distance > 3)
				return false;
			else {
				affectedUser.push_back(sock);
				return true;
			}
		}
	}
	return true;

	*/
}

