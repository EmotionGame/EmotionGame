#include "stdafx.h"
#include "NetworkEngine.h"

NetworkEngine::NetworkEngine()
{
}

NetworkEngine::~NetworkEngine()
{
}

bool NetworkEngine::Initialize(HWND hwnd, char* pIP, char* pPort)
{
#ifdef _DEBUG
	printf("Start >> NetworkEngine.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;

	m_IP = pIP;
	m_Port = pPort;

	m_SendUserPacketQueue = new std::queue<UserPacket>;
	if (!m_SendUserPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendUserPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvUserPacketQueue = new std::queue<UserPacket>;
	if (!m_RecvUserPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvActionPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_SendActionPacketQueue = new std::queue<ActionPacket>;
	if (!m_SendActionPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendActionPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvActionPacketQueue = new std::queue<ActionPacket>;
	if (!m_RecvActionPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvActionPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_SendEventPacketQueue = new std::queue<EventPacket>;
	if (!m_SendEventPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendEventPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvEventPacketQueue = new std::queue<EventPacket>;
	if (!m_RecvEventPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvEventPacketQueue", L"Error", MB_OK);
		return false;
	}

	// 서버와 연결하지 않았을 때 테스트용
	UserPacket test;
	test.id = 10;
	m_RecvUserPacketQueue->push(test);
	m_ConnectFlag = true;

	/***** Winsock 초기화 *****/
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : WSAStartup(MAKEWORD(2, 2), &m_wsaData)", L"Error", MB_OK);
		return false;
	}

	/***** 소켓 생성 *****/
	m_hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_hSocket == INVALID_SOCKET)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);", L"Error", MB_OK);
		return false;
	}

	// 주소 구조체 설정
	memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
	m_ServerAddr.sin_family = AF_INET;

	// IP 주소 변환 함수 : 문자열 형태로 IP 주소를 입력받아 32비트 숫자로 리턴
	// 이런 함수는 기본적으로 바이트 오더 변환 기능이 내장되어 있음
	//m_ServerAddr.sin_addr.s_addr = inet_addr(argv[1]);
	inet_pton(AF_INET, m_IP, &m_ServerAddr.sin_addr.s_addr);
	m_ServerAddr.sin_port = htons(atoi(m_Port));

	Connect();

#ifdef _DEBUG
	printf("Success >> NetworkEngine.cpp : Initialize()\n");
#endif

	return true;
}

void NetworkEngine::Shutdown()
{
	if (m_SendEventPacketQueue)
	{
		delete m_SendEventPacketQueue;
		m_SendEventPacketQueue = nullptr;
	}
	if (m_RecvEventPacketQueue)
	{
		delete m_RecvEventPacketQueue;
		m_RecvEventPacketQueue = nullptr;
	}
	if (m_SendActionPacketQueue)
	{
		delete m_SendActionPacketQueue;
		m_SendActionPacketQueue = nullptr;
	}
	if (m_RecvActionPacketQueue)
	{
		delete m_RecvActionPacketQueue;
		m_RecvActionPacketQueue = nullptr;
	}

	if (m_ConnectFlag)
	{
		CloseHandle(m_hSendThread);
		CloseHandle(m_hRecvThread);

		closesocket(m_hSocket);
	}

	WSACleanup();
}

bool NetworkEngine::Frame()
{
	if (!m_ConnectFlag)
	{
		Connect();
	}

	return true;
}

bool NetworkEngine::Connect()
{
	//printf("Start >> NetworkEngine.cpp : Connect()\n");

	//if (WSAConnect(m_hSocket, (SOCKADDR*)&m_ServerAddr, sizeof(m_ServerAddr), &m_ConnectBuff, NULL, NULL, NULL) == SOCKET_ERROR)
	//{
	//	printf("Fail >> NetworkEngine.cpp : Connect()\n");
	//}
	//else
	//{
	//	printf("Success >> NetworkEngine.cpp : Connect()\n");

	//	m_ConnectFlag = true;

	//	// 구조체에 이벤트 핸들을 삽입해서 전달
	//	m_Event = WSACreateEvent();
	//	memset(&m_Overlapped, 0, sizeof(m_Overlapped));
	//	m_Overlapped.hEvent = m_Event;

	//	// Thread 구동
	//	m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, (LPVOID)this, 0, NULL);
	//	m_hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, (LPVOID)this, 0, NULL);
	//}

	return true;
}

unsigned int __stdcall NetworkEngine::SendThread(void* p)
{
	NetworkEngine* read = static_cast<NetworkEngine*>(p);
	read->_SendThread();

	return true;
}
UINT WINAPI NetworkEngine::_SendThread()
{
	while (1)
	{
		ActionPacket tempAP;
		EventPacket tempEP;

		/***** ActionPacket : 시작 *****/
		/***** m_SendAPQueueMutex : 시작 *****/
		m_SendAPQueueMutex.lock();

		if (!m_SendActionPacketQueue->empty())
		{
			tempAP = m_SendActionPacketQueue->front();
			m_SendActionPacketQueue->pop();

			m_SendAPQueueMutex.unlock();
			/***** m_SendAPQueueMutex : 종료 *****/

			printf("Pop >> NetworkEngine.cpp : ActionPacket -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
				tempAP.type,
				tempAP.id,
				tempAP.position[0],
				tempAP.position[1],
				tempAP.position[2],
				tempAP.rotation[0],
				tempAP.rotation[1],
				tempAP.rotation[2]
			);

			m_SendBuff.len = sizeof(ActionPacket);
			m_SendBuff.buf = (char*)&tempAP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
				printf("Fail >> NetworkEngine.cpp : ActionPacket -> WSASend\n");
				continue;
			}
			printf("Success >> NetworkEngine.cpp : ActionPacket -> WSASend\n");

			/***** m_SendAPQueueMutex : 시작 *****/
			m_SendAPQueueMutex.lock();
		}

		m_SendAPQueueMutex.unlock();
		/***** m_SendAPQueueMutex : 종료 *****/
		/***** ActionPacket : 종료 *****/

		/***** EventPacket : 시작 *****/
		/***** m_SendEPQueueMutex : 시작 *****/
		m_SendEPQueueMutex.lock();

		if (!m_SendEventPacketQueue->empty())
		{
			tempEP = m_SendEventPacketQueue->front();
			m_SendEventPacketQueue->pop();

			m_SendEPQueueMutex.unlock();
			/***** m_SendEPQueueMutex : 종료 *****/

			printf("Pop >> NetworkEngine.cpp : EventPacket -> type = %d, id = %d, position = {%f %f %f}, state = %s\n",
				tempEP.type,
				tempEP.id,
				tempEP.position[0],
				tempEP.position[1],
				tempEP.position[2],
				tempEP.state ? "true" : "false"
			);

			m_SendBuff.len = sizeof(EventPacket);
			m_SendBuff.buf = (char*)&tempEP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
				printf("Fail >> NetworkEngine.cpp : EventPacket -> WSASend\n");
				continue;
			}
			printf("Success >> NetworkEngine.cpp : EventPacket -> WSASend\n");

			/***** m_SendEPQueueMutex : 시작 *****/
			m_SendEPQueueMutex.lock();
		}

		m_SendEPQueueMutex.unlock();
		/***** m_SendEPQueueMutex : 종료 *****/
		/***** EventPacket : 시작 *****/

	}

	_endthreadex(0);
}
unsigned int __stdcall NetworkEngine::RecvThread(void* p)
{
	NetworkEngine* read = static_cast<NetworkEngine*>(p);
	read->_RecvThread();

	return true;
}
UINT WINAPI NetworkEngine::_RecvThread()
{
	char buffer[256];

	UserPacket* tempUP;
	ActionPacket* tempAP;
	EventPacket* tempEP;

	while (1)
	{
		tempUP = nullptr;
		tempAP = nullptr;
		tempEP = nullptr;

		if (recv(m_hSocket, buffer, 256, 0) == SOCKET_ERROR)
		{
			printf("Fail >> NetworkEngine.cpp : recv\n");
			continue;
		}
		printf("Success >> NetworkEngine.cpp : recv\n");

		int* type = reinterpret_cast<int*>(buffer);


		switch (*type)
		{
		case 11: // ActionPacket, 데이터 변경을 위한 type
			// 접속한 플레이어의 값 설정을 위해
			tempAP = reinterpret_cast<ActionPacket*>(buffer);

			/***** m_RecvAPQueueMutex : 시작 *****/
			m_RecvAPQueueMutex.lock();

			if (m_RecvActionPacketQueue->size() < QUEUE_LIMIT_SIZE)
			{
				m_RecvActionPacketQueue->push(*tempAP);
#ifdef _DEBUG
				m_RecvAPQueueMutex.unlock();
				/***** m_RecvAPQueueMutex : 종료 *****/

				printf("Pop >> NetworkEngine.cpp : ActionPacket -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
					tempAP->type,
					tempAP->id,
					tempAP->position[0],
					tempAP->position[1],
					tempAP->position[2],
					tempAP->rotation[0],
					tempAP->rotation[1],
					tempAP->rotation[2]
				);

				/***** m_RecvAPQueueMutex : 시작 *****/
				m_RecvAPQueueMutex.lock();
#endif
			}

			m_RecvAPQueueMutex.unlock();
			/***** m_RecvAPQueueMutex : 종료 *****/

			break;

		case 21: // EventPacket
			/***** m_RecvEPQueueMutex : 시작 *****/
			m_RecvEPQueueMutex.lock();

			if (m_RecvEventPacketQueue->size() < QUEUE_LIMIT_SIZE)
			{
				tempEP = reinterpret_cast<EventPacket*>(buffer);

				m_RecvEventPacketQueue->push(*tempEP);
#ifdef _DEBUG
				m_RecvEPQueueMutex.unlock();
				/***** m_RecvEPQueueMutex : 종료 *****/

				printf("Push >> NetworkEngine.cpp : EventPacket -> type = %d, id = %d, position = {%f %f %f}, state = %s\n",
					tempEP->type,
					tempEP->id,
					tempEP->position[0],
					tempEP->position[1],
					tempEP->position[2],
					tempEP->state ? "true" : "false"
				);

				/***** m_RecvEPQueueMutex : 시작 *****/
				m_RecvEPQueueMutex.lock();
#endif
			}
			m_RecvEPQueueMutex.unlock();
			/***** m_RecvEPQueueMutex : 종료 *****/

			break;

		case 30: // UserPacket, 자신을 등록하기 위한 type
		case 31: // UserPacket, 타인들을 등록하기 위한 type
			tempUP = reinterpret_cast<UserPacket*>(buffer);

			/***** m_RecvUPQueueMutex : 시작 *****/
			m_RecvUPQueueMutex.lock();

			if (m_RecvUserPacketQueue->size() < QUEUE_LIMIT_SIZE)
			{
				m_RecvUserPacketQueue->push(*tempUP);
#ifdef _DEBUG
				m_RecvUPQueueMutex.unlock();
				/***** m_RecvUPQueueMutex : 종료 *****/

				printf("Pop >> NetworkEngine.cpp : UserPacket -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
					tempUP->type,
					tempUP->id,
					tempUP->position[0],
					tempUP->position[1],
					tempUP->position[2],
					tempUP->rotation[0],
					tempUP->rotation[1],
					tempUP->rotation[2]
				);

				/***** m_RecvUPQueueMutex : 시작 *****/
				m_RecvUPQueueMutex.lock();
#endif
			}

			m_RecvUPQueueMutex.unlock();
			/***** m_RecvUPQueueMutex : 종료 *****/

			break;
		}

	}

	_endthreadex(0);
}

bool NetworkEngine::GetConnectFlag()
{
	return m_ConnectFlag;
}

std::queue<UserPacket>* NetworkEngine::GetSendUPQueue()
{
	return m_SendUserPacketQueue;
}
std::queue<UserPacket>* NetworkEngine::GetRecvUPQueue()
{
	return m_RecvUserPacketQueue;
}
std::mutex& NetworkEngine::GetSendUPQueueMutex()
{
	return m_SendUPQueueMutex;
}
std::mutex& NetworkEngine::GetRecvUPQueueMutex()
{
	return m_RecvUPQueueMutex;
}

std::queue<ActionPacket>* NetworkEngine::GetSendAPQueue()
{
	return m_SendActionPacketQueue;
}
std::queue<ActionPacket>* NetworkEngine::GetRecvAPQueue()
{
	return m_RecvActionPacketQueue;
}
std::mutex& NetworkEngine::GetSendAPQueueMutex()
{
	return m_SendAPQueueMutex;
}
std::mutex& NetworkEngine::GetRecvAPQueueMutex()
{
	return m_RecvAPQueueMutex;
}

std::queue<EventPacket>* NetworkEngine::GetSendEPQueue()
{
	return m_SendEventPacketQueue;
}
std::queue<EventPacket>* NetworkEngine::GetRecvEPQueue()
{
	return m_RecvEventPacketQueue;
}
std::mutex& NetworkEngine::GetSendEPQueueMutex()
{
	return m_SendEPQueueMutex;
}
std::mutex& NetworkEngine::GetRecvEPQueueMutex()
{
	return m_RecvEPQueueMutex;
}