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

	m_SendUserPacket30Queue = new std::queue<UserPacket>;
	if (!m_SendUserPacket30Queue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendUserPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvUserPacket30Queue = new std::queue<UserPacket>;
	if (!m_RecvUserPacket30Queue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvActionPacketQueue", L"Error", MB_OK);
		return false;
	}

	m_SendUserPacket31Queue = new std::queue<UserPacket>;
	if (!m_SendUserPacket31Queue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendUserPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvUserPacket31Queue = new std::queue<UserPacket>;
	if (!m_RecvUserPacket31Queue)
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

	m_SendEventAcquirePacketQueue = new std::queue<EventAcquirePacket>;
	if (!m_SendEventAcquirePacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendEventAcquirePacketQueue", L"Error", MB_OK);
		return false;
	}

	m_SendMonsterPacketQueue = new std::queue<MonsterPacket>;
	if (!m_SendMonsterPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendMonsterPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvMonsterPacketQueue = new std::queue<MonsterPacket>;
	if (!m_RecvMonsterPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvMonsterPacketQueue", L"Error", MB_OK);
		return false;
	}

	m_SendMonsterAttackPacketQueue = new std::queue<MonsterAttackPacket>;
	if (!m_SendMonsterAttackPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendMonsterPacketQueue", L"Error", MB_OK);
		return false;
	}

	m_SendObjectPacketQueue = new std::queue<ObejctPacket>;
	if (!m_SendObjectPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendObjectPacketQueue", L"Error", MB_OK);
		return false;
	}
	m_RecvObjectPacketQueue = new std::queue<ObejctPacket>;
	if (!m_RecvObjectPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvObjectPacketQueue", L"Error", MB_OK);
		return false;
	}

	m_SendPlayer2PlayerQueue = new std::queue<Player2Player>;
	if (!m_SendPlayer2PlayerQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendPlayer2PlayerQueue", L"Error", MB_OK);
		return false;
	}

	m_SendPlayer2MonsterQueue = new std::queue<Player2Monster>;
	if (!m_SendPlayer2MonsterQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendPlayer2MonsterQueue", L"Error", MB_OK);
		return false;
	}

	m_SendPlayer2ObjectQueue = new std::queue<Player2Object>;
	if (!m_SendPlayer2ObjectQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_SendPlayer2ObjectQueue", L"Error", MB_OK);
		return false;
	}

	m_RecvGameOverPacketQueue = new std::queue<GameOverPacket>;
	if (!m_RecvGameOverPacketQueue)
	{
		MessageBox(m_hwnd, L"NetworkEngine.cpp : m_RecvGameOverPacketQueue", L"Error", MB_OK);
		return false;
	}

	///**** 서버와 연결하지 않았을 때 테스트용 : 시작 *****/
	//UserPacket UPtest;
	//for (int i = 0; i < 4; i++)
	//{
	//	UPtest.id = i;
	//	int x = 16 * (i % 100) + 104;
	//	int z = 5 * (i / 100) + 80;
	//	UPtest.position[0] = 1.0f * x;
	//	UPtest.position[1] = 5.0f;
	//	UPtest.position[2] = 1.0f * z;
	//	m_RecvUserPacket30Queue->push(UPtest);
	//}
	//for (int i = 0; i < 4; i++)
	//{
	//	UPtest.id = i + 4;
	//	int x = 16 * (i % 100) + 104;
	//	int z = 5 * (i / 100) + 176;
	//	UPtest.position[0] = 1.0f * x;
	//	UPtest.position[1] = 5.0f;
	//	UPtest.position[2] = 1.0f * z;
	//	UPtest.rotation[1] = 180.0f;
	//	m_RecvUserPacket30Queue->push(UPtest);
	//}

	//EventPacket EPtest;
	//for (int i = 1; i <= 8; i++)
	//{
	//	if (i != 5)
	//	{
	//		EPtest.id = i;
	//		int x = 16 * (i % 100) + 72;
	//		int z = 5 * (i / 100) + 128;
	//		EPtest.position[0] = 1.0f * x;
	//		EPtest.position[1] = 5.0f;
	//		EPtest.position[2] = 1.0f * z;
	//		EPtest.state = true;
	//		m_RecvEventPacketQueue->push(EPtest);
	//	}
	//}


	//m_ConnectFlag = true;
	///**** 서버와 연결하지 않았을 때 테스트용 : 종료 *****/


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
#ifdef _DEBUG
	printf("Start >> NetworkEngine.cpp : Connect()\n");
#endif

	if (WSAConnect(m_hSocket, (SOCKADDR*)&m_ServerAddr, sizeof(m_ServerAddr), &m_ConnectBuff, NULL, NULL, NULL) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		printf("Fail >> NetworkEngine.cpp : Connect()\n");
#endif
	}
	else
	{
#ifdef _DEBUG
		printf("Success >> NetworkEngine.cpp : Connect()\n");
#endif
		m_ConnectFlag = true;

		// 구조체에 이벤트 핸들을 삽입해서 전달
		m_Event = WSACreateEvent();
		memset(&m_Overlapped, 0, sizeof(m_Overlapped));
		m_Overlapped.hEvent = m_Event;

		// Thread 구동
		m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, (LPVOID)this, 0, NULL);
		m_hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, (LPVOID)this, 0, NULL);
	}

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
		EventAcquirePacket tempEAP;
		UserPacket tempUP;
		MonsterPacket tempMP;
		MonsterAttackPacket tempMAP;
		Player2Player tempP2P;
		Player2Monster tempP2M;
		Player2Object tempP2O;

		/***** ActionPacket : 시작 *****/
		/***** m_SendAPQueueMutex : 시작 *****/
		m_SendAPQueueMutex.lock();
		if (!m_SendActionPacketQueue->empty())
		{
			tempAP = m_SendActionPacketQueue->front();
			m_SendActionPacketQueue->pop();
			m_SendAPQueueMutex.unlock();
			/***** m_SendAPQueueMutex : 종료 *****/
#ifdef _DEBUG
			/*printf("Pop >> NetworkEngine.cpp : ActionPacket -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
				tempAP.type,
				tempAP.id,
				tempAP.position[0],
				tempAP.position[1],
				tempAP.position[2],
				tempAP.rotation[0],
				tempAP.rotation[1],
				tempAP.rotation[2]
			);*/
#endif
			m_SendBuff.len = sizeof(ActionPacket);
			m_SendBuff.buf = (char*)&tempAP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : ActionPacket -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : ActionPacket -> WSASend\n");
#endif

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
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : EventPacket -> type = %d, id = %d, position = {%f %f %f}, state = %s\n",
				tempEP.type,
				tempEP.id,
				tempEP.position[0],
				tempEP.position[1],
				tempEP.position[2],
				tempEP.state ? "true" : "false"
			);
#endif
			m_SendBuff.len = sizeof(EventPacket);
			m_SendBuff.buf = (char*)&tempEP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : EventPacket -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : EventPacket -> WSASend\n");
#endif
			/***** m_SendEPQueueMutex : 시작 *****/
			m_SendEPQueueMutex.lock();
		}
		m_SendEPQueueMutex.unlock();
		/***** m_SendEPQueueMutex : 종료 *****/
		/***** EventPacket : 종료 *****/


		/***** EventAcquirePacket : 시작 *****/
		/***** m_SendEAPQueueMutex : 시작 *****/
		m_SendEAPQueueMutex.lock();
		if (!m_SendEventAcquirePacketQueue->empty())
		{
			tempEAP = m_SendEventAcquirePacketQueue->front();
			m_SendEventAcquirePacketQueue->pop();
			m_SendEAPQueueMutex.unlock();
			/***** m_SendEAPQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : EventAcquirePacket -> type = %d, eventId = %d, playerId = %d, position = (%f %f %f)\n",
				tempEAP.type,
				tempEAP.eventId,
				tempEAP.playerId,
				tempEAP.position[0],
				tempEAP.position[1],
				tempEAP.position[2]
			);
#endif
			m_SendBuff.len = sizeof(EventAcquirePacket);
			m_SendBuff.buf = (char*)&tempEAP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : EventAcquirePacket -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : EventAcquirePacket -> WSASend\n");
#endif
			/***** m_SendEAPQueueMutex : 시작 *****/
			m_SendEAPQueueMutex.lock();
		}
		m_SendEAPQueueMutex.unlock();
		/***** m_SendEAPQueueMutex : 종료 *****/
		/***** EventAcquirePacket : 종료 *****/
		

		/***** UserPacket : 시작 *****/
		/***** m_SendUP31QueueMutex : 시작 *****/
		m_SendUP31QueueMutex.lock();
		if (!m_SendUserPacket31Queue->empty())
		{
			tempUP = m_SendUserPacket31Queue->front();
			m_SendUserPacket31Queue->pop();
			m_SendUP31QueueMutex.unlock();
			/***** m_SendUP31QueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : UserPacket31 -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
				tempUP.type,
				tempUP.id,
				tempUP.position[0],
				tempUP.position[1],
				tempUP.position[2],
				tempUP.rotation[0],
				tempUP.rotation[1],
				tempUP.rotation[2]
			);
#endif
			m_SendBuff.len = sizeof(UserPacket);
			m_SendBuff.buf = (char*)&tempUP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : UserPacket31 -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : UserPacket31 -> WSASend\n");
#endif

			/***** m_SendUP31QueueMutex : 시작 *****/
			m_SendUP31QueueMutex.lock();
		}
		m_SendUP31QueueMutex.unlock();
		/***** m_SendUP31QueueMutex : 종료 *****/
		/***** UserPacket : 종료 *****/


		/***** MonsterPacket : 시작 *****/
		/***** m_SendMPQueueMutex : 시작 *****/
		m_SendMPQueueMutex.lock();
		if (!m_SendMonsterPacketQueue->empty())
		{
			tempMP = m_SendMonsterPacketQueue->front();
			m_SendMonsterPacketQueue->pop();
			m_SendMPQueueMutex.unlock();
			/***** m_SendMPQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : MonsterPacket -> type = %d, position = {%f %f %f}\n",
				tempMP.type,
				tempMP.position[0],
				tempMP.position[1],
				tempMP.position[2]
			);
#endif
			m_SendBuff.len = sizeof(MonsterPacket);
			m_SendBuff.buf = (char*)&tempMP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : MonsterPacket -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : MonsterPacket -> WSASend\n");
#endif
			/***** m_SendMPQueueMutex : 시작 *****/
			m_SendMPQueueMutex.lock();
		}
		m_SendMPQueueMutex.unlock();
		/***** m_SendMPQueueMutex : 종료 *****/
		/***** MonsterPacket : 종료 *****/


		/***** MonsterAttackPacket : 시작 *****/
		/***** m_SendMAPQueueMutex : 시작 *****/
		m_SendMAPQueueMutex.lock();
		if (!m_SendMonsterAttackPacketQueue->empty())
		{
			tempMAP = m_SendMonsterAttackPacketQueue->front();
			m_SendMonsterAttackPacketQueue->pop();
			m_SendMAPQueueMutex.unlock();
			/***** m_SendMAPQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : MonsterAttackPacket -> type = %d, playerId = %d, position = {%f %f %f}, collision = %s\n",
				tempMAP.type,
				tempMAP.playerId,
				tempMAP.position[0],
				tempMAP.position[1],
				tempMAP.position[2],
				tempMAP.collision ? "true" : "false"
			);
#endif
			m_SendBuff.len = sizeof(MonsterAttackPacket);
			m_SendBuff.buf = (char*)&tempMAP;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : MonsterAttackPacket -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : MonsterAttackPacket -> WSASend\n");
#endif
			/***** m_SendMAPQueueMutex : 시작 *****/
			m_SendMAPQueueMutex.lock();
		}
		m_SendMAPQueueMutex.unlock();
		/***** m_SendMAPQueueMutex : 종료 *****/
		/***** MonsterAttackPacket : 종료 *****/

		/***** Player2Player : 시작 *****/
		/***** m_SendP2PQueueMutex : 시작 *****/
		m_SendP2PQueueMutex.lock();
		if (!m_SendPlayer2PlayerQueue->empty())
		{
			tempP2P = m_SendPlayer2PlayerQueue->front();
			m_SendPlayer2PlayerQueue->pop();
			m_SendP2PQueueMutex.unlock();
			/***** m_SendP2PQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : Player2Player -> type = %d, playerId1 = %d, playerId2 = %d, emotion = (%d, %d, %d, %d)\n",
				tempP2P.type,
				tempP2P.player1Id,
				tempP2P.player2Id,
				tempP2P.emotion[0],
				tempP2P.emotion[1],
				tempP2P.emotion[2],
				tempP2P.emotion[3]
			);
#endif
			m_SendBuff.len = sizeof(Player2Player);
			m_SendBuff.buf = (char*)&tempP2P;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : Player2Player -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : Player2Player -> WSASend\n");
#endif
			/***** m_SendP2PQueueMutex : 시작 *****/
			m_SendP2PQueueMutex.lock();
		}
		m_SendP2PQueueMutex.unlock();
		/***** m_SendP2PQueueMutex : 종료 *****/
		/***** Player2Player : 종료 *****/

		/***** Player2Monster : 시작 *****/
		/***** m_SendP2MQueueMutex : 시작 *****/
		m_SendP2MQueueMutex.lock();
		if (!m_SendPlayer2MonsterQueue->empty())
		{
			tempP2M = m_SendPlayer2MonsterQueue->front();
			m_SendPlayer2MonsterQueue->pop();
			m_SendP2MQueueMutex.unlock();
			/***** m_SendP2MQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : Player2Monster -> type = %d, emotion = (%d, %d, %d, %d)\n",
				tempP2M.type,
				tempP2M.emotion[0],
				tempP2M.emotion[1],
				tempP2M.emotion[2],
				tempP2M.emotion[3]
			);
#endif
			m_SendBuff.len = sizeof(Player2Monster);
			m_SendBuff.buf = (char*)&tempP2M;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : Player2Monster -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : Player2Monster -> WSASend\n");
#endif
			/***** m_SendP2MQueueMutex : 시작 *****/
			m_SendP2MQueueMutex.lock();
		}
		m_SendP2MQueueMutex.unlock();
		/***** m_SendP2MQueueMutex : 종료 *****/
		/***** Player2Monster : 종료 *****/

		/***** Player2Object : 시작 *****/
		/***** m_SendP2OQueueMutex : 시작 *****/
		m_SendP2OQueueMutex.lock();
		if (!m_SendPlayer2ObjectQueue->empty())
		{
			tempP2O = m_SendPlayer2ObjectQueue->front();
			m_SendPlayer2ObjectQueue->pop();
			m_SendP2OQueueMutex.unlock();
			/***** m_SendP2OQueueMutex : 종료 *****/
#ifdef _DEBUG
			printf("Pop >> NetworkEngine.cpp : Player2Object -> type = %d, objectId = %d, emotion = (%d, %d, %d, %d)\n",
				tempP2O.type,
				tempP2O.objectId,
				tempP2O.emotion[0],
				tempP2O.emotion[1],
				tempP2O.emotion[2],
				tempP2O.emotion[3]
			);
#endif
			m_SendBuff.len = sizeof(Player2Object);
			m_SendBuff.buf = (char*)&tempP2O;

			if (WSASend(m_hSocket, &m_SendBuff, 1, (LPDWORD)&m_SendBytes, 0, &m_Overlapped, NULL) == SOCKET_ERROR)
			{
#ifdef _DEBUG
				printf("Fail >> NetworkEngine.cpp : Player2Object -> WSASend\n");
#endif
				continue;
			}
#ifdef _DEBUG
			printf("Success >> NetworkEngine.cpp : Player2Object -> WSASend\n");
#endif
			/***** m_SendP2OQueueMutex : 시작 *****/
			m_SendP2OQueueMutex.lock();
		}
		m_SendP2OQueueMutex.unlock();
		/***** m_SendP2OQueueMutex : 종료 *****/
		/***** Player2Object : 종료 *****/
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
	MonsterPacket* tempMP;
	ObejctPacket* tempOP;
	GameOverPacket* tempGOP;

	while (1)
	{
		tempUP = nullptr;
		tempAP = nullptr;
		tempEP = nullptr;
		tempMP = nullptr;
		tempOP = nullptr;

		DWORD recvByte = 0;
		DWORD offset = 0;

		if ((recvByte = recv(m_hSocket, buffer, 256, 0)) == SOCKET_ERROR)
		{
#ifdef _DEBUG
			printf("Fail >> NetworkEngine.cpp : recv\n");
#endif
			continue;
		}
#ifdef _DEBUG
		printf("Success >> NetworkEngine.cpp : recv\n");
#endif

		while (recvByte - offset > 0)
		{
			int* type = reinterpret_cast<int*>(buffer + offset);

			switch (*type)
			{
			case 11: // ActionPacket, 데이터 변경을 위한 type
				// 접속한 플레이어의 값 설정을 위해
				tempAP = reinterpret_cast<ActionPacket*>(buffer + offset);

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

				offset += sizeof(ActionPacket);
				break;

			case 21: // EventPacket
					 /***** m_RecvEPQueueMutex : 시작 *****/
				m_RecvEPQueueMutex.lock();

				if (m_RecvEventPacketQueue->size() < QUEUE_LIMIT_SIZE)
				{
					tempEP = reinterpret_cast<EventPacket*>(buffer + offset);

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

				offset += sizeof(EventPacket);
				break;

			case 30: // UserPacket30, 초기화를 위한 type
				tempUP = reinterpret_cast<UserPacket*>(buffer + offset);

				/***** m_RecvUPQueueMutex : 시작 *****/
				m_RecvUP30QueueMutex.lock();

				if (m_RecvUserPacket30Queue->size() < QUEUE_LIMIT_SIZE)
				{
					m_RecvUserPacket30Queue->push(*tempUP);
#ifdef _DEBUG
					m_RecvUP30QueueMutex.unlock();
					/***** m_RecvUPQueueMutex : 종료 *****/

					printf("Pop >> NetworkEngine.cpp : UserPacket30 -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}, emotion = (%d, %d, %d, %d), hp = %d\n",
						tempUP->type,
						tempUP->id,
						tempUP->position[0],
						tempUP->position[1],
						tempUP->position[2],
						tempUP->rotation[0],
						tempUP->rotation[1],
						tempUP->rotation[2],
						tempUP->emotion[0],
						tempUP->emotion[1],
						tempUP->emotion[2],
						tempUP->emotion[3],
						tempUP->hp
					);

					/***** m_RecvUPQueueMutex : 시작 *****/
					m_RecvUP30QueueMutex.lock();
#endif
				}
				m_RecvUP30QueueMutex.unlock();
				/***** m_RecvUPQueueMutex : 종료 *****/
				offset += sizeof(UserPacket);
				break;

			case 31: // UserPacket31, 값 변경을 위한 type
				tempUP = reinterpret_cast<UserPacket*>(buffer + offset);

				/***** m_RecvUPQueueMutex : 시작 *****/
				m_RecvUP31QueueMutex.lock();

				if (m_RecvUserPacket31Queue->size() < QUEUE_LIMIT_SIZE)
				{
					m_RecvUserPacket31Queue->push(*tempUP);
#ifdef _DEBUG
					m_RecvUP31QueueMutex.unlock();
					/***** m_RecvUPQueueMutex : 종료 *****/

					printf("Pop >> NetworkEngine.cpp : UserPacket31 -> type = %d, id = %d, position = {%f %f %f}, rotation = {%f %f %f}, emotion = (%d, %d, %d, %d)\n",
						tempUP->type,
						tempUP->id,
						tempUP->position[0],
						tempUP->position[1],
						tempUP->position[2],
						tempUP->rotation[0],
						tempUP->rotation[1],
						tempUP->rotation[2],
						tempUP->emotion[0],
						tempUP->emotion[1],
						tempUP->emotion[2],
						tempUP->emotion[3]
					);

					/***** m_RecvUPQueueMutex : 시작 *****/
					m_RecvUP31QueueMutex.lock();
#endif
				}
				m_RecvUP31QueueMutex.unlock();
				/***** m_RecvUPQueueMutex : 종료 *****/

				offset += sizeof(UserPacket);
				break;

			case 50: // MonsterPacket
				tempMP = reinterpret_cast<MonsterPacket*>(buffer + offset);

				/***** m_RecvMPQueueMutex : 시작 *****/
				m_RecvMPQueueMutex.lock();

				if (m_RecvMonsterPacketQueue->size() < QUEUE_LIMIT_SIZE)
				{
					m_RecvMonsterPacketQueue->push(*tempMP);
#ifdef _DEBUG
					m_RecvMPQueueMutex.unlock();
					/***** m_RecvMPQueueMutex : 종료 *****/

					//printf("Pop >> NetworkEngine.cpp : MonsterPacket -> type = %d, position = {%f %f %f}, rotation = {%f %f %f}\n",
					//	tempMP->type,
					//	tempMP->position[0],
					//	tempMP->position[1],
					//	tempMP->position[2],
					//	tempMP->rotation[0],
					//	tempMP->rotation[1],
					//	tempMP->rotation[2]
					//);

					/***** m_RecvMPQueueMutex : 시작 *****/
					m_RecvMPQueueMutex.lock();
#endif
				}
				m_RecvMPQueueMutex.unlock();
				/***** m_RecvMPQueueMutex : 종료 *****/

				offset += sizeof(MonsterPacket);
				break;

			case 100: // GameOverPacket
				tempGOP = reinterpret_cast<GameOverPacket*>(buffer + offset);

				/***** m_RecvGOPQueueMutex : 시작 *****/
				m_RecvGOPQueueMutex.lock();

				if (m_RecvGameOverPacketQueue->size() < QUEUE_LIMIT_SIZE)
				{
					m_RecvGameOverPacketQueue->push(*tempGOP);
#ifdef _DEBUG
					m_RecvGOPQueueMutex.unlock();
					/***** m_RecvGOPQueueMutex : 종료 *****/

					printf("GameOverPacket Recv!\n");

					/***** m_RecvGOPQueueMutex : 시작 *****/
					m_RecvGOPQueueMutex.lock();
#endif
				}
				m_RecvGOPQueueMutex.unlock();
				/***** m_RecvGOPQueueMutex : 종료 *****/

				offset += sizeof(GameOverPacket);
				break;

//			case 40:
//				tempOP = reinterpret_cast<ObejctPacket*>(buffer + offset);
//
//				/***** m_RecvOPQueueMutex : 시작 *****/
//				m_RecvOPQueueMutex.lock();
//
//				if (m_RecvObjectPacketQueue->size() < QUEUE_LIMIT_SIZE)
//				{
//					m_RecvObjectPacketQueue->push(*tempOP);
//#ifdef _DEBUG
//					m_RecvOPQueueMutex.unlock();
//					/***** m_RecvOPQueueMutex : 종료 *****/
//
//					printf("Pop >> NetworkEngine.cpp : ObejctPacket -> type = %d, id = %d, position = {%f %f %f}\n",
//						tempOP->type,
//						tempOP->id,
//						tempOP->position[0],
//						tempOP->position[1],
//						tempOP->position[2]
//					);
//
//					/***** m_RecvOPQueueMutex : 시작 *****/
//					m_RecvOPQueueMutex.lock();
//#endif
//				}
//				m_RecvOPQueueMutex.unlock();
//				/***** m_RecvOPQueueMutex : 종료 *****/
//
//				offset += sizeof(ObejctPacket);
//				break;
			}
		}
	}

	_endthreadex(0);
}

bool NetworkEngine::GetConnectFlag()
{
	return m_ConnectFlag;
}

std::queue<UserPacket>* NetworkEngine::GetSendUP30Queue()
{
	return m_SendUserPacket30Queue;
}
std::queue<UserPacket>* NetworkEngine::GetRecvUP30Queue()
{
	return m_RecvUserPacket30Queue;
}
std::mutex& NetworkEngine::GetSendUP30QueueMutex()
{
	return m_SendUP30QueueMutex;
}
std::mutex& NetworkEngine::GetRecvUP30QueueMutex()
{
	return m_RecvUP30QueueMutex;
}

std::queue<UserPacket>* NetworkEngine::GetSendUP31Queue()
{
	return m_SendUserPacket31Queue;
}
std::queue<UserPacket>* NetworkEngine::GetRecvUP31Queue()
{
	return m_RecvUserPacket31Queue;
}
std::mutex& NetworkEngine::GetSendUP31QueueMutex()
{
	return m_SendUP31QueueMutex;
}
std::mutex& NetworkEngine::GetRecvUP31QueueMutex()
{
	return m_RecvUP31QueueMutex;
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

std::queue<EventAcquirePacket>* NetworkEngine::GetSendEAPQueue()
{
	return m_SendEventAcquirePacketQueue;
}
std::mutex& NetworkEngine::GetSendEAPQueueMutex()
{
	return m_SendEAPQueueMutex;
}

std::queue<MonsterPacket>* NetworkEngine::GetSendMPQueue()
{
	return m_SendMonsterPacketQueue;
}
std::queue<MonsterPacket>* NetworkEngine::GetRecvMPQueue()
{
	return m_RecvMonsterPacketQueue;
}
std::mutex& NetworkEngine::GetSendMPQueueMutex()
{
	return m_SendMPQueueMutex;
}
std::mutex& NetworkEngine::GetRecvMPQueueMutex()
{
	return m_RecvMPQueueMutex;
}

std::queue<MonsterAttackPacket>* NetworkEngine::GetSendMAPQueue()
{
	return m_SendMonsterAttackPacketQueue;
}
std::mutex& NetworkEngine::GetSendMAPQueueMutex()
{
	return m_SendMAPQueueMutex;
}

std::queue<ObejctPacket>* NetworkEngine::GetSendOPQueue()
{
	return m_SendObjectPacketQueue;
}
std::queue<ObejctPacket>* NetworkEngine::GetRecvOPQueue()
{
	return m_RecvObjectPacketQueue;
}
std::mutex& NetworkEngine::GetSendOPQueueMutex()
{
	return m_SendOPQueueMutex;
}
std::mutex& NetworkEngine::GetRecvOPQueueMutex()
{
	return m_RecvOPQueueMutex;
}

std::queue<Player2Player>* NetworkEngine::GetSendP2PQueue()
{
	return m_SendPlayer2PlayerQueue;
}
std::mutex& NetworkEngine::GetSendP2PQueueMutex()
{
	return m_SendP2PQueueMutex;
}

std::queue<Player2Monster>* NetworkEngine::GetSendP2MQueue()
{
	return m_SendPlayer2MonsterQueue;
}
std::mutex& NetworkEngine::GetSendP2MQueueMutex()
{
	return m_SendP2MQueueMutex;
}

std::queue<Player2Object>* NetworkEngine::GetSendP2OQueue()
{
	return m_SendPlayer2ObjectQueue;
}
std::mutex& NetworkEngine::GetSendP2OQueueMutex()
{
	return m_SendP2OQueueMutex;
}

std::queue<GameOverPacket>* NetworkEngine::GetRecvGOPQueue()
{
	return m_RecvGameOverPacketQueue;
}
std::mutex& NetworkEngine::GetRecvGOPQueueMutex()
{
	return m_RecvGOPQueueMutex;
}