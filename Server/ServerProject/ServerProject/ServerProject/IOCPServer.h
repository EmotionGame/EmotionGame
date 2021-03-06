#pragma once

#include <iostream>

#include <process.h>
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>

#include <map>
#include <vector>
#include <list>
#include <queue>
#include <mutex>
#include <atomic>
//#include <tbb/concurrent_queue.h>

#include "Deflag.h"
#include "UserManager.h"
#include "EventManager.h"
#include "Object.h"
#include "MonsterManager.h"
//#include "Terrain.h"
//#include "QuadTree.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

typedef struct {
	SOCKET sock;
	SOCKADDR_IN sockAddr;
	// bool connected
}PER_HANDLE_DATA, *LPER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	// 전송할 데이터X / 저장할 최대 버퍼 사이즈
	char buffer[BUFSIZE];
	int operationType;
	SOCKET sock;
}PER_IO_DATA, *LPER_IO_DATA;

class IOCPServer {
public:
	IOCPServer();
	~IOCPServer();

	bool init();
	bool setPool();
	void cleanUp();

	bool setSocket();
	// static - _beginthreadex() 함수의 인자 때문, _completionThread() 호출 후 completionThread() 맴버 함수 호출
	static unsigned int __stdcall _completionThread(void *p_this);
	UINT WINAPI completionThread();

	// Send to All Users, Data-Type and char*
	void broadcastSend(int type, int i) {
		for (auto iter = users.begin(); iter != users.end(); iter++) {
			LPER_IO_DATA ov = new PER_IO_DATA();
			memset(&(ov->overlapped), 0, sizeof(OVERLAPPED));
			memset(&(ov->buffer), 0, BUFSIZE);
			ov->operationType = WRITE;
			DWORD dwSend = 0;
			if (type == EVENT_PACKET) {
				char* eventData = NULL;
				eventData = _eventManager->getEvent(i);
				//memcpy(ov->buffer, eventData, sizeof(EventPacket));
				ov->wsaBuf.len = sizeof(EventPacket);
				ov->wsaBuf.buf = (char*)eventData;
				WSASend(iter->first, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
			}
			else if (type == USER_UPDATE_PACKET) {
				char* user_update_data = _userManager->getUserInfo(i);
				User* user_data = reinterpret_cast<User*>(user_update_data);
				user_data->type = USER_UPDATE_PACKET;
				//memcpy(ov->buffer, user_data, sizeof(User));
				ov->wsaBuf.len = sizeof(User);
				ov->wsaBuf.buf = (char*)user_data;
				WSASend(iter->first, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
			}
			else if (type == MONSTER_PACKET) {
				char* data = _monsterManager->getMonsterInfo();
				ov->wsaBuf.len = sizeof(Monster);
				ov->wsaBuf.buf = (char*)data;
				WSASend(iter->first, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
			}
		}
	}

protected:
	int count = 0;
	map<SOCKET, int> users;
	vector<User*> userInfo;
	// API
	LPFN_ACCEPTEX lpfnAcceptEx;
	// 확인용 mutex

	bool gameStart = false;
	// Fixed Event Object
	bool gameSetting = false;

	EventManager* _eventManager = NULL;
	UserManager* _userManager = NULL;
	Object* _object = NULL;
	MonsterManager* _monsterManager = NULL;

	clock_t tic;
	// Monster 주기
	clock_t tok=clock(), tek = clock();

	atomic<bool> eventFlag = true;
	atomic<bool> MonsterFlag = true;
	atomic<bool> secFlag = true;
private:
	SOCKET mServerSock;
	SOCKADDR_IN mServerSockAddr;

	std::vector<SOCKET> socketPool;
	std::vector<LPER_IO_DATA> ovPool;
	char exBuf[25];
	HANDLE mCompletionPort;
};

void ErrorHandling(LPCSTR errorMsg);