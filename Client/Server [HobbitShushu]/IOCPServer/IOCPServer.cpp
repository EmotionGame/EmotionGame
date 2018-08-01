// IOCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#include "stdafx.h"

#include "IOCPServer.h"
#include "PacketManager.h"
#include "EventManager.h"

#define PORT 8090

using namespace std;

int main()
{
	IOCPServer* server = new IOCPServer();
	server->init();
	server->setPool();
	Sleep(INFINITE);
	
	delete server;

    return 0;
}

inline IOCPServer::IOCPServer()
{
	mServerSock = NULL;
	mCompletionPort = NULL;
	userID = 0;
	InitializeCriticalSection(&mSc);
	
	lpfnAcceptEx = NULL;
}

IOCPServer::~IOCPServer()
{
	DeleteCriticalSection(&mSc);
	closesocket(mServerSock);
	WSACleanup();
}

void IOCPServer::cleanUp()
{
	// PQCS
}
bool IOCPServer::init()
{
	WSAData wsaData;
	SYSTEM_INFO systemInfo;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() Error");

	// Completion Port init
	// 마지막 인자는 Completion Port에 대한 Thread의 수로 프로세스 하나당 하나의 Thread를 두는것이 Context Switching을 피하는 방법이다
	// 0 - CPU 프로세스 하나당 하나의 Thread 생성
	mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&systemInfo);

	for (int i = 0; i < systemInfo.dwNumberOfProcessors+2; i++)
		_beginthreadex(NULL, 0, _completionThread, (LPVOID)this, 0, NULL);

	_beginthreadex(NULL, 0, _workerThread, (LPVOID)this, 0, NULL);

	setSocket();
	return true;
}

bool IOCPServer::setPool()
{
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes;

	// AcceptEx를 메모리에 등록
	WSAIoctl(mServerSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&bytes, NULL, NULL
	);

	for (short i = 0; i < POOLSIZE; i++)
	{
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET)
			ErrorHandling("client pool socket error");
		DWORD zero = 0;
		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero));
		socketPool.push_back(sock);

		// overlapped struct 초기화
		LPER_IO_DATA pIoData = new PER_IO_DATA;
		memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
		pIoData->wsaBuf.len = PACKETSIZE;
		pIoData->wsaBuf.buf = pIoData->buffer;
		pIoData->operationType = ACCEPT;
		pIoData->sock = sock;
		ovPool.push_back(pIoData);

		memset(exBuf, 0, sizeof(exBuf));

		lpfnAcceptEx(mServerSock, sock, exBuf,
			//((BUFSIZE-sizeof(SOCKADDR_IN)+16)*2),
			0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
			&bytes, &(ovPool[i]->overlapped)
		);
		if (WSAGetLastError() != ERROR_IO_PENDING || WSAGetLastError() == WSAECONNRESET)
			ErrorHandling("Not Pending Error");
		//
		cout << sock << endl;
	}
	return true;
}

// 사용중 X
/*
bool IOCPServer::acceptQueue()
{
lpfnAcceptEx = NULL;
DWORD bytes;
while (1)
{
for (int i = 0; i < POOLSIZE; i++) {
// if not connected
// overlapped 초기화
// acceptex 함수 실행
if (socketPool[i] == NULL) {
if (GetLastError() != WSA_IO_PENDING) { cout << "AcceptQueue Error"<<endl; exit(1); }
cout << &socketPool[i] << " : " << socketPool[i] << endl;
cout << &ovPool[i] << " : " << &ovPool[i]->overlapped << endl;

memset(&(ovPool[i]->overlapped), 0, sizeof(OVERLAPPED));
ovPool[i]->wsaBuf.len = PACKETSIZE;
ovPool[i]->wsaBuf.buf = ovPool[i]->buffer;
ovPool[i]->operationType = ACCEPT;

memset(exBuf, 0, sizeof(exBuf));

lpfnAcceptEx(mServerSock, socketPool[i], exBuf,
0,
sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
&bytes, &(ovPool[i]->overlapped)
);
if (WSAGetLastError() != ERROR_IO_PENDING || WSAGetLastError() == WSAECONNRESET)
ErrorHandling("Not Pending Error");
}
}
}
return true;
}

*/

bool IOCPServer::setSocket()
{
	bool optval = true;
	// server socket setting
	mServerSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mServerSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() Error");
	setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	// Bind IO Device(Listen Socket) and Completion Port
	CreateIoCompletionPort((HANDLE)mServerSock, mCompletionPort, (DWORD)ACCEPT, 0);

	// SOCKADDR_IN setting
	memset(&mServerSockAddr, 0, sizeof(SOCKADDR_IN));
	mServerSockAddr.sin_family = AF_INET;
	mServerSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mServerSockAddr.sin_port = htons(PORT);
	
	// bind
	if (::bind(mServerSock, (SOCKADDR*)&mServerSockAddr, sizeof(mServerSockAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() Error");

	bool on = true;
	if (setsockopt(mServerSock, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char *)&on, sizeof(on)))
		return -1;

	// listen
	if (listen(mServerSock, 10) == SOCKET_ERROR)
		ErrorHandling("listen() Error");

	return true;
}

// __stdcall : wdinwos api에서 사용하고 함수에서 스택을 관리하는 기법
unsigned int __stdcall IOCPServer::_completionThread(void * p_this)
{
	IOCPServer* pIOCPServer = static_cast<IOCPServer*>(p_this);
	pIOCPServer->completionThread();
	return 0;
}
unsigned int __stdcall IOCPServer::_userThread(void *p_this)
{
	IOCPServer* pIOCPServer = static_cast<IOCPServer*>(p_this);
	pIOCPServer->userThread();
	return 0;
}
unsigned int __stdcall IOCPServer::_eventThread(void * p_this)
{
	IOCPServer* pIOCPServer = static_cast<IOCPServer*>(p_this);
	pIOCPServer->eventThread();
	return 0;
}
unsigned int __stdcall IOCPServer::_workerThread(void * p_this)
{
	IOCPServer* pIOCPServer = static_cast<IOCPServer*>(p_this);
	pIOCPServer->workerThread();
	return 0;
}
// 유저가 가지는 Buffer를 순회하고 Data를 송신하는 Thread
UINT WINAPI IOCPServer::userThread()
{
	unsigned int id = (unsigned int&)std::this_thread::get_id();
	SOCKET userSock = NULL;
	while (1) {
		userSock = _uThread[id];
		if (userSock)
			break;
	}
	// user buffer 순회
	// data 있으면 user socket에 send
	while (1)
	{
		while (_uBuffer[userSock].empty()) {}
		WSABUF data = _uBuffer[userSock].front();
		_uBuffer[userSock].pop_front();
		onSend(userSock, data);
	}
	return true;
}
UINT WINAPI IOCPServer::eventThread()
{
	_eventManager = new EventManager();
	// eventQueue_ 에 event 초기값 저장
	_eventManager->init();
	while (1)
	{
		while (users.empty()) {}
		if (_eventManager->empty()) { 
			// empty면 리소스 정리 필요 / 혹은 해제
			delete _eventManager;
			// Thread도 종료(본인)
			break;
		}
		else {
			EventPacket* event = _eventManager->getEvent();
			// 반환값이 없으면 0초가 없는것
			if (event == NULL) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				_eventManager->tic(100);
			}
			else {
				EnterCriticalSection(&mSc);
				_buffer.push_back(make_pair(BROADCAST, (char*)event));
				LeaveCriticalSection(&mSc);
			}
		}
	}
	cout << "Event Buf size " << _eventBuf.size() << endl;
	return true;
}
UINT WINAPI IOCPServer::workerThread()
{
	while (1)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(12));
		while (users.empty() == true) {}
		while (_buffer.empty()) {}

		SOCKET sock = _buffer.front().first;
		char* buf = _buffer.front().second;
		_buffer.pop_front();

		int* type = (int*)buf;	
		switch (*type)
		{
		case ACTION_PACKET:
		{
			cout << "ACTION_PACKET" << endl;
			ActionPacket* ac = reinterpret_cast<ActionPacket*>(buf);
			WSABUF wsaBuf;
			wsaBuf.len = sizeof(ActionPacket);
			wsaBuf.buf = reinterpret_cast<char*>(ac);

			cout << ac->id << endl;
			cout << ac->position[0] << " " << ac->position[1] << " " << ac->position[2] << endl;
			cout << ac->rotation[0] << " " << ac->rotation[1] << " " << ac->rotation[2] << endl;
			
			// broadcast user id and pos of the user
			if (sock == BROADCAST) {
				for (auto iter = users.begin(); iter != users.end(); iter++)
					// 원래는 여기서 USER struct 생성 및 송신
					_uBuffer[iter->first].push_back(wsaBuf);
					//onSend(iter->first, wsaBuf);
				// & 다른 유저의 정보를 접속한 유저에게 송신
				for (auto iter = users.begin(); iter != users.end(); iter++) {
					if (ac->id != iter->first) {
						WSABUF userData;
						userData.len = sizeof(ActionPacket);
						ActionPacket temp = iter->second;
						temp.type = 12;
						userData.buf = (char*)&temp;
						//userData.buf = (char*)&iter->second;
						_uBuffer[iter->first].push_back(wsaBuf);
						//onSend(ac->id, userData);
					}
				}
			}
			// 나머진 자신을 제외한 user에게 send
			else {
				// User - Action - UserData update - Broadcast to other users
				for (int i = 0; i < 3; i++) {
					users[sock].position[i] = ac->position[i];
					users[sock].rotation[i] = ac->rotation[i];
				}
				//users[sock].position
				for (auto iter = users.begin(); iter != users.end(); iter++)
					if (sock != iter->first)
						_uBuffer[iter->first].push_back(wsaBuf);
						//onSend(iter->first, wsaBuf);
			}
			break;
		}
		case EVENT_PACKET:
		{
			cout << "EVENT_PACKET" << endl;
			EventPacket* data = reinterpret_cast<EventPacket*>(buf);
			WSABUF wsaBuf;
			wsaBuf.len = sizeof(EventPacket);
			wsaBuf.buf = reinterpret_cast<char*>(data);
			if (sock == BROADCAST) {
				for (auto iter = users.begin(); iter != users.end(); iter++)
					_uBuffer[iter->first].push_back(wsaBuf);
					//onSend(iter->first, wsaBuf);
			}
		}
		break;
		default:break;
		}
	}
	return true;

}
UINT WINAPI IOCPServer::completionThread()
{
	HANDLE hCompletionPort = mCompletionPort;
	DWORD dwBytesTransferred;
	LPER_HANDLE_DATA pHandleData;
	LPER_IO_DATA pIoData;
	DWORD dwFlag = 0;

	PACKET* packet = NULL;

	// For Disconnect
	SOCKADDR* lpaddr = NULL, *lpremote = NULL;
	int lplen=0, lprelen=0;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
	LPFN_DISCONNECTEX lpfnDisconnectEx = NULL;
	GUID GuidDisconnect = WSAID_DISCONNECTEX;
	DWORD bytes;

	PacketManager* pm = new PacketManager();
	while (TRUE)
	{
		GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred, (LPDWORD)&pHandleData, (LPOVERLAPPED*)&pIoData, INFINITE);
		
		if (pHandleData == NULL) {
			cout << "pHandleData = NULL";
			exit(1);
		}

		// CK - FLAG
		switch ((int)pHandleData)
		{
		case ACCEPT:
		{
			cout << "pIoData->operationType : ACCEPT" << endl;
			pHandleData = new PER_HANDLE_DATA();
			pHandleData->sock = pIoData->sock;

			// 0727 : 유저 Thread 생성/적용
			unsigned int threadID;
			_beginthreadex(NULL, 0, _userThread, (LPVOID)this, 0, &threadID);
			_uThread[threadID] = pHandleData->sock;

			userID++;
			cout << "----------------------CURRENT USER COUNT  " << userID << "   ----------------------" <<endl;
			if (userID == 8) {
				cout << "Event Thread Start" << endl;
				_beginthreadex(NULL, 0, _eventThread, (LPVOID)this, 0, NULL);
			}
			// 미포함 : GetAddress
			// AcceptEx 함수 종료 - 접속 된 socket을 CP에 등록
			CreateIoCompletionPort((HANDLE)pHandleData->sock, mCompletionPort, (DWORD)pHandleData, 0);

			// 초기 데이터 설정 ( 접속 시 id 및 위치 )
			pActionPacket initPacket = new ActionPacket();
			initPacket->type = ACTION_PACKET;
			initPacket->id = pHandleData->sock;
			for (int i = 0; i < 3; i++) {
				initPacket->position[0] = 1.0+i;
				initPacket->position[1] = 0.0;
				initPacket->position[2] = 1.0+i;
				initPacket->rotation[i] = 0.0;
			}
			// users<SOCKET, USER>
			users.insert(pair<SOCKET, ActionPacket>(pHandleData->sock, (ActionPacket&)initPacket));
			
			// 0727
			EnterCriticalSection(&mSc);
			_buffer.push_back(make_pair(BROADCAST, (char*)initPacket));
			LeaveCriticalSection(&mSc);

			memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
			memset(&(pIoData->buffer), 0, sizeof(PACKET));
			pIoData->wsaBuf.len = PACKETSIZE;
			pIoData->wsaBuf.buf = pIoData->buffer;
			pIoData->operationType = READ;
			dwFlag = 0;
			WSARecv(pHandleData->sock, &(pIoData->wsaBuf), 1, &dwBytesTransferred, &dwFlag, &(pIoData->overlapped), NULL);
			continue;
		}
		case QUIT:
			// Thread 종료
			continue;
		}
		if (dwBytesTransferred == 0 && pIoData->operationType != DISCONNECT) {
			cout << "0BYTE - Disconnect" << endl;
			// Load Disconnect
			WSAIoctl(pHandleData->sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidDisconnect, sizeof(GuidDisconnect),
				&lpfnDisconnectEx, sizeof(lpfnDisconnectEx),
				&bytes, NULL, NULL
			);		
			memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
			pIoData->operationType = DISCONNECT;
			pIoData->sock = pHandleData->sock;
			if (lpfnDisconnectEx(pIoData->sock, &pIoData->overlapped, TF_REUSE_SOCKET, 0) == false) {
				if (GetLastError() != ERROR_IO_PENDING) {
					// 후처리 필요
					ErrorHandling("Disconnect Error");
				}
			}
			break;
		}
		else if (pIoData->operationType == DISCONNECT) {
			cout << "pIoData->operationType : ACCEPTEX" << endl;
			// 소켓은 재사용 가능하게 해제되었고 Overlapped만 Disconnect 상태
			// 소켓 재사용 및 AcceptEx 사용
			// Overlapped 초기화
			
			// 해당 socket을 key로 가지는 데이터 삭제
			users.erase(pIoData->sock);

			memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
			memset(&(pIoData->buffer), 0, sizeof(PACKET));
			pIoData->wsaBuf.len = PACKETSIZE;
			pIoData->wsaBuf.buf = pIoData->buffer;
			pIoData->operationType = ACCEPT;
			lpfnAcceptEx(mServerSock, pIoData->sock, exBuf,
				0,
				sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
				&bytes, &(pIoData->overlapped)
			);
			cout << "접속한 유저 수 : " << users.size() << endl;
			continue;
		}

		cout << "GET BYTE : " << dwBytesTransferred << endl;
		int offset = 0;
		while (dwBytesTransferred - offset > 0)
		{
			switch (pIoData->operationType)
			{
			case READ:
			{
				// buffer의 4바이트를 읽어서 type을 확인
				SOCKET sock = pHandleData->sock;
				//int packetType = (int&)pIoData->buffer + offset;
				int* packetType = (int*)&pIoData->buffer[offset];
				cout << "pIoData->operationType : READ" << endl;
				char* buf = NULL;

				switch (*packetType)
				{
				case ACTION_PACKET:
				{
					char* buf = new char[sizeof(ActionPacket)];
					memcpy(buf, pIoData->buffer + offset, sizeof(ActionPacket));
					// 0727
					EnterCriticalSection(&mSc);
					_buffer.push_back(make_pair(pHandleData->sock, buf));
					LeaveCriticalSection(&mSc);
					offset += sizeof(ActionPacket);
					break;
				}
				default:
					cout << "-----------------------------DEFAULT TYPE-----------------------------" << endl;
					// 타입 11 외 다른타입 전송시 기존 데이터 전송중
					break;
				}
			}
			break;
			default: cout << "DEFAULT" << endl;
				break;
			}
		}
		// 새로운 overlapped 구조체를 생성하지 않고 그대로 재사용
		memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
		memset(&(pIoData->buffer), 0, PACKETSIZE);
		pIoData->wsaBuf.len = PACKETSIZE;
		pIoData->operationType = READ;
		pIoData->sock = pHandleData->sock;
		dwFlag = 0;
		WSARecv(pHandleData->sock, &(pIoData->wsaBuf), 1, NULL, &dwFlag, &(pIoData->overlapped), NULL);
	}
	return 0;
}



void ErrorHandling(LPCSTR errorMsg)
{
	fputs(errorMsg, stderr);
	fputc('\n', stderr);
	exit(1);
}
