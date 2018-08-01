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
	// ������ ������X / ������ �ִ� ���� ������
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

	// static - _beginthreadex() �Լ��� ���� ����, _completionThread() ȣ�� �� completionThread() �ɹ� �Լ� ȣ��
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
	// ������ ����
	list<pair<SOCKET, char*>> _buffer; // WHO, DATA

	EventManager* _eventManager;
	vector<PACKET*> _eventBuf;
	// Make User count ���߿� ���� Ŭ������ ���� �ű�� �͵� ����
	int userID = 0;
	
	// ������ ���� (USER)
	map<SOCKET, pUser>users; // WHO, USER DATA
	// Ŭ�󸶴� ������ Buffer
	tbb::concurrent_queue<char*> _userQueue[8];
	
	// API
	LPFN_ACCEPTEX lpfnAcceptEx;
	int sendCount = 0;
	// Ȯ�ο� mutex
	mutex g_mutex;

	bool gameStart = false;
};

void ErrorHandling(LPCSTR errorMsg);