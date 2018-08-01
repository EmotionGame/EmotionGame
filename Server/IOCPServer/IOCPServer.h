#pragma once

#include <iostream>

#include <process.h>
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <map>
#include <list>
#include <atomic>

#include "Deflag.h"
#include "PacketManager.h"
#include "EventManager.h"

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
	char buffer[PACKETSIZE];
	int operationType;
	SOCKET sock;
	// int type -> packet
}PER_IO_DATA, *LPER_IO_DATA;

class IOCPServer {
public:
	IOCPServer();
	~IOCPServer();

	bool init();
	bool setPool();
	void cleanUp();

	//bool acceptQueue();

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

	void syncUserBuffer(bool flag, SOCKET sock, WSABUF data)
	{
		EnterCriticalSection(&mUserSc);
		// true: push
		if (flag)
			_uBuffer[sock].push_back(data);
		// false: pop
		else
			_uBuffer[sock].pop_front();
		LeaveCriticalSection(&mUserSc);
	}

	void syncBuffer(bool flag, pair<SOCKET, char*> pData)
	{
		EnterCriticalSection(&mSc);
		// true: push
		if (flag) 
			_buffer.push_back(pData);
		// false: pop
		else 
			_buffer.pop_front();
		LeaveCriticalSection(&mSc);
	}

	// static - _beginthreadex() �Լ��� ���� ����, _completionThread() ȣ�� �� completionThread() �ɹ� �Լ� ȣ��
	static unsigned int __stdcall _completionThread(void *p_this);
	UINT WINAPI completionThread();
	static unsigned int __stdcall _eventThread(void* p_this);
	UINT WINAPI eventThread();
	static unsigned int __stdcall _workerThread(void* p_this);
	UINT WINAPI workerThread();
	// 0727
	static unsigned int __stdcall _userThread(void* p_this);
	UINT WINAPI userThread();
private:
	SOCKET mServerSock;
	SOCKADDR_IN mServerSockAddr;

	// use pair?
	vector<SOCKET> socketPool;
	vector<LPER_IO_DATA> ovPool;
	char exBuf[256];
	HANDLE mCompletionPort;

	// Server - Logic
	list<pair<SOCKET, char*>> _buffer; // WHO, DATA

	EventManager* _eventManager;
	vector<PACKET*> _eventBuf;
	// Make User count ���߿� ���� Ŭ������ ���� �ű�� �͵� ����
	int userID = 0;
	// ������ ���� (USER)
	map<SOCKET, pUser>users; // WHO, USER DATA
	// �����帶�� �ϳ��� ������ ó���ϱ� ����, ���� �����, Thread ����ó��
	map<unsigned, SOCKET> _uThread;
	//map<SOCKET, HANDLE> _uThreadHandle;
	// ���� �����, k-v ����, list ó��������ϳ�?
	map<SOCKET, list<WSABUF>> _uBuffer;
	// API
	LPFN_ACCEPTEX lpfnAcceptEx;

	int sendCount = 0;
	CRITICAL_SECTION mSc;
	CRITICAL_SECTION mUserSc;

	bool gameStart = false;
};

void ErrorHandling(LPCSTR errorMsg);