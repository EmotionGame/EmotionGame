#include "stdafx.h"
#include "EventManager.h"
// Thread

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
	for (int i = 0; i < eventQueue_.size(); i++)
		free(eventQueue_[i].eventPacket);
	eventQueue_.clear();
}

bool EventManager::init()
{
	// 몬스터가 오브젝트 공격 or 패턴이 변경되는 시점
	srand(time(NULL));
	EventInfo eventInfo;
	EventPacket* event = new EventPacket;
	event->type = EVENT_PACKET;
	event->id = 9;

	event->position[0] = 0;
	event->position[1] = 0;
	event->position[2] = 0;
	event->state = true;

	eventInfo.time = 2 * 60 * 1000; // 2min
	eventInfo.eventPacket = event;
	eventQueue_.push_back(eventInfo);

	// Random Event
	event = randomEvent(1);
	pushEvent(event, 10);
	event = randomEvent(2);
	pushEvent(event, 13);
	event = randomEvent(3);
	pushEvent(event, 1);
	event = randomEvent(4);
	pushEvent(event, 1);
	event = randomEvent(6);
	pushEvent(event, 1);
	event = randomEvent(7);
	pushEvent(event, 15);
	event = randomEvent(8);
	pushEvent(event, 17);

	return true;
}

EventPacket* EventManager::randomEvent(int id)
{
	EventPacket* random = new EventPacket;
	random->type = EVENT_PACKET;
	random->id = id;
	for (int i = 0; i < 3; i++) {
		int rnd = rand() % 171;
		random->position[0] = randomValue(clock() + rnd);
		random->position[1] = 0;
		random->position[2] = randomValue(clock() + rnd);
	}
	if (id == 3 || id == 4 || id == 6)
		random->state = true;
	else
		random->state = false;

	return random;
}

void EventManager::pushEvent(EventPacket * event, float time)
{
	EventInfo info;
	info.time = time * 1000;
	info.eventPacket = event;
	eventQueue_.push_back(info);
}
int EventManager::size()
{
	return eventQueue_.size();
}
float EventManager::getEventTime(int i)
{
	// for loop를 통해 접근 - 0부터
	return eventQueue_[i].time;
}
char* EventManager::getEvent(int i)
{
	char* event = reinterpret_cast<char*>(eventQueue_[i].eventPacket);
	return event;
}
int* EventManager::getEmotion(int i)
{
	int* emotion = new int[4];
	for (int i = 0; i < 4; i++)
		emotion[i] = 0;

	if (i == 1 || i == 2)
		emotion[0] = 20;
	else if (i == 3 || i == 4)
		emotion[1] = 3;
	else if (i == 5)
		emotion[2] = 3;
	else if (i == 6 || i == 7)
		emotion[3] = 20;
	return emotion;
}
bool EventManager::validate(int i, float pos[3])
{
	lock_guard<mutex>lock(g_mutex);
	clock_t tic = clock();
	float distance = sqrt(pow(eventQueue_[i].eventPacket->position[0] - pos[0], 2)
		+ pow(eventQueue_[i].eventPacket->position[2] - pos[2], 2));
	cout << "Event Validate>>Distance>>" << distance << endl;
	cout << "Event Validate>>i>>" << i << endl;
	if (distance <= 256 && eventQueue_[i].eventPacket->state)
		return true;

	return false;
}
void EventManager::setEvent(EventPacket* event)
{
	int id = event->id;
	for (int i = 0; i < 3; i++)
		eventQueue_[id].eventPacket->position[i] = event->position[i];
}
void EventManager::updateEvent(int i, clock_t time)
{
	lock_guard<mutex>lock(g_mutex);
	switch (i)
	{
	case 1: case 2: case 6: case 7:
	{
		if (eventQueue_[i].eventPacket->state == false) {
			// 이벤트 발생
			eventQueue_[i].time = time + (5 * 1000);
			eventQueue_[i].eventPacket->state = true;
			cout << "EVENT UPDATE>>eventPacket->state i>>" << eventQueue_[i].eventPacket->state << endl;
		}
		else {
			// 사라짐
			eventQueue_[i].time = time + (10 * 1000);
			eventQueue_[i].eventPacket->state = false;
			eventQueue_[i].eventPacket->position[0] = randomValue(time);
			eventQueue_[i].eventPacket->position[2] = randomValue(time);
			cout << "EVENT UPDATE>>eventPacket->state i>>" << eventQueue_[i].eventPacket->state << endl;
		}
		break;
	}
	case 3: case 4: case 5:
	{
		// 고정 이벤트
		eventQueue_[i].time = time + 1000;
		break;
	}
	case 0:
		// 몬스터가 오브젝트 공격 시작
		eventQueue_[i].time = time * 1000;
		//eventQueue_[i].eventPacket->state = false;
		break;
	}
}
bool EventManager::userInEvent(int i, float pos[3])
{
	// 해당 이벤트와 거리 비교
	float distance = sqrt(pow(eventQueue_[i].eventPacket->position[0] - pos[0], 2)
		+ pow(eventQueue_[i].eventPacket->position[1] - pos[1], 2)
		+ pow(eventQueue_[i].eventPacket->position[2] - pos[2], 2));
	if (eventQueue_[i].eventPacket->state && distance <= 3.0) {
		return true;
	}
	return false;
}

int EventManager::randomValue(clock_t t)
{
	int random;
	//< 1단계. 시드 설정
	random_device rn;
	mt19937_64 rnd(rn());

	//< 2단계. 분포 설정 ( 정수 )
	uniform_int_distribution<int> range(128, 188);
	random = range(rnd);
	srand(t);
	if (rand() % 2 == 0)
		random *= -1;
	return random;
}
