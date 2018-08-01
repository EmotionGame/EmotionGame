#pragma once

#include <iostream>

#include <process.h>
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <map>
#include <list>
#include <atomic>
#include <mutex>
#include <tbb/concurrent_queue.h>

#include "Deflag.h"
#include "PacketManager.h"
#include "EventManager.h"
#include "UserManager.h"

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
	
	//tbb::concurrent_queue queue;
	
	bool setSocket();

	bool onSend(SOCKET cSock, WSABUF wsaBuf)
	{
		DWORD byte;
		if (WSASend(cSock, &wsaBuf, 1, &byte, 0, NULL, NULL) == SOCKET_ERROR) {
			cout << _buffer.size() << endl;
			cout << "*******************************************Send Error*******************************************" << endl;
		}
		cout << "onSend Function / Send to : " << cSock << "  Count : " << ++sendCount << endl;
		return true;
	}

	SOCKET getFilledSock()
	{
	}

	void syncBuffer(bool flag, pair<SOCKET, char*> pData)
	{
		// true: push
		if (flag) 
			_buffer.push_back(pData);
		// false: pop
		else if(flag == false && !_buffer.empty())
			_buffer.pop_front();
	}

	// static - _beginthreadex() 함수의 인자 때문, _completionThread() 호출 후 completionThread() 맴버 함수 호출
	static unsigned int __stdcall _completionThread(void *p_this);
	UINT WINAPI completionThread();
private:
	SOCKET mServerSock;
	SOCKADDR_IN mServerSockAddr;

	// use pair?
	vector<SOCKET> socketPool;
	vector<LPER_IO_DATA> ovPool;
	char exBuf[25];
	HANDLE mCompletionPort;

	// Server - Logic
	// 제외할 예정
	list<pair<SOCKET, char*>> _buffer; // WHO, DATA

	EventManager* _eventManager;
	vector<PACKET*> _eventBuf;
	// Make User count 나중에 유저 클래스를 만들어서 옮기는 것도 생각
	int userID = 0;
	
	// 원래는 유저 (USER)
	map<SOCKET, pUser>users; // WHO, USER DATA
	// 클라마다 가지는 Buffer
	tbb::concurrent_queue<char*> _userQueue[8];
	
	// API
	LPFN_ACCEPTEX lpfnAcceptEx;
	int sendCount = 0;
	// 확인용 mutex
	mutex g_mutex;

	bool gameStart = false;
};

void ErrorHandling(LPCSTR errorMsg);