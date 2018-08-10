#include "stdafx.h"
#include "IOCPServer.h"
#include "Deflag.h"

#define PORT 8090

using namespace std;

IOCPServer::IOCPServer()
{
	mServerSock = NULL;
	mCompletionPort = NULL;
	count = 0;
	lpfnAcceptEx = NULL;
	_userManager = new UserManager();
	_eventManager = new EventManager();
	_object = new Object();
	_monsterManager = new MonsterManager(_userManager);
	/*
	m_Terrain = new Terrain;
	m_QuadTree = new QuadTree;

	m_Terrain->Initialize("Terrain/heightmap01.bmp");

	m_QuadTree->Initialize(m_Terrain);

	m_QuadTree->GetHeightAtPosition(x, z, y);
	*/
}

IOCPServer::~IOCPServer()
{
	/*
	if (m_QuadTree)
	{
		m_QuadTree->Shutdown();
		delete m_QuadTree;
		m_QuadTree = 0;
	}

	if (m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	*/
	
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
	// ������ ���ڴ� Completion Port�� ���� Thread�� ���� ���μ��� �ϳ��� �ϳ��� Thread�� �δ°��� Context Switching�� ���ϴ� ����̴�
	// 0 - CPU ���μ��� �ϳ��� �ϳ��� Thread ����
	mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&systemInfo);

	for (int i = 0; i < systemInfo.dwNumberOfProcessors * 2; i++)
		_beginthreadex(NULL, 0, &IOCPServer::_completionThread, (LPVOID)this, 0, NULL);

	setSocket();

	_eventManager->init();
	return true;
}

bool IOCPServer::setPool()
{
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes;

	// AcceptEx�� �޸𸮿� ���
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

		// overlapped struct �ʱ�ȭ
		LPER_IO_DATA pIoData = new PER_IO_DATA;
		memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
		pIoData->wsaBuf.len = BUFSIZE;
		pIoData->wsaBuf.buf = pIoData->buffer;
		pIoData->operationType = ACCEPT;
		pIoData->sock = sock;
		ovPool.push_back(pIoData);

		memset(exBuf, 0, sizeof(exBuf));

		lpfnAcceptEx(mServerSock, sock, exBuf,
			0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
			&bytes, &(ovPool[i]->overlapped)
		);
		if (WSAGetLastError() != ERROR_IO_PENDING || WSAGetLastError() == WSAECONNRESET)
			ErrorHandling("Not Pending Error");
	}
	return true;
}

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

// __stdcall : wdinwos api���� ����ϰ� �Լ����� ������ �����ϴ� ���
unsigned int __stdcall IOCPServer::_completionThread(void * p_this)
{
	IOCPServer* pIOCPServer = static_cast<IOCPServer*>(p_this);
	pIOCPServer->completionThread();
	return 0;
}
UINT WINAPI IOCPServer::completionThread()
{
	HANDLE hCompletionPort = mCompletionPort;
	DWORD dwBytesTransferred = 0;
	LPER_HANDLE_DATA pHandleData;
	LPER_IO_DATA pIoData = NULL;
	DWORD dwFlag = 0;

	bool timeout = false;

	// For Disconnect
	SOCKADDR* lpaddr = NULL, *lpremote = NULL;
	int lplen = 0, lprelen = 0;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
	LPFN_DISCONNECTEX lpfnDisconnectEx = NULL;
	GUID GuidDisconnect = WSAID_DISCONNECTEX;
	DWORD bytes;

	vector<int> jobQueue;
	
	bool eventExpected = true;
	bool monsterExpected = true;
	while (TRUE)
	{
		timeout = GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred,
			(LPDWORD)&pHandleData, (LPOVERLAPPED*)&pIoData, 1);
		if (timeout) {
			// Data ����
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

				// ������ : GetAddress
				// AcceptEx �Լ� ���� - ���� �� socket�� CP�� ���
				CreateIoCompletionPort((HANDLE)pHandleData->sock, mCompletionPort, (DWORD)pHandleData, 0);

				// ������ ������ ������ UserManager�� ���� ����
				int id = _userManager->enterUser(pHandleData->sock);
				users.insert({ pHandleData->sock, id });
				char* data = _userManager->getUserInfo(id);
				if (!data)
					ErrorHandling("getUserInfo Error");
				userInfo.push_back((User*)data);

				// �ڽſ��� ������ ���� ���� ���� & ���� �����ڿ��� �ڽ��� ���� ����
				for (auto iter = users.begin(); iter != users.end(); iter++) {
					if (iter->first != pHandleData->sock) {
						_userManager->setJob(iter->second, data);
						jobQueue.push_back(iter->second);
						_userManager->setJob(id, _userManager->getUserInfo(iter->second));
						jobQueue.push_back(id);
					}
				}
				memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
				memset(&(pIoData->buffer), 0, BUFSIZE);
				pIoData->wsaBuf.len = BUFSIZE;
				pIoData->wsaBuf.buf = pIoData->buffer;
				pIoData->operationType = READ;
				pIoData->sock = pHandleData->sock;
				dwFlag = 0;
				WSARecv(pHandleData->sock, &(pIoData->wsaBuf), 1, NULL, &dwFlag, &(pIoData->overlapped), NULL);
				
				count++;
				cout << "----------------------CURRENT USER COUNT  " << count << "   ----------------------" << endl;
				if (count == USERSIZE) {
					gameStart = true;
					tic = clock();
					if (!gameSetting) {
						for (int i = 3; i <= 5; i++) 
							broadcastSend(EVENT_PACKET, i);
						gameSetting = true;
					}
					_monsterManager->start();
					broadcastSend(MONSTER_PACKET, 0);
				}
				// �ʱ� ���� �� �ٷ� �ڽ��� ���̵� ����
				pIoData = new PER_IO_DATA();
				memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
				memset(pIoData->buffer, 0, sizeof(BUFSIZE));
				memcpy(pIoData->buffer, data, sizeof(User));
				pIoData->wsaBuf.len = sizeof(User);
				pIoData->wsaBuf.buf = pIoData->buffer;
				pIoData->operationType = WRITE;
				dwFlag = 0;
				WSASend(pHandleData->sock, &pIoData->wsaBuf, 1, &dwBytesTransferred, 0, NULL, NULL);
				continue;
			}
			case QUIT:
				// Thread ����
				continue;
			}

			// 0 Byte�ε� Disconnect �Լ� ȣ�� �� - ��
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
						// ��ó�� �ʿ�
						ErrorHandling("Disconnect Error");
					}
				}
				break;
			}
			else if (pIoData->operationType == DISCONNECT) {
				cout << "pIoData->operationType : ACCEPTEX" << endl;
				// ������ ���� �����ϰ� �����Ǿ��� Overlapped�� Disconnect ����
				// ���� ���� �� AcceptEx ���
				// Overlapped �ʱ�ȭ
				//users.erase(pIoData->sock);
				//_userManager->exitUser(pIoData->sock, users[pIoData->sock]);

				// overlapped �ʱ�ȭ �� 
				memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
				memset(&(pIoData->buffer), 0, sizeof(BUFSIZE));
				pIoData->wsaBuf.len = BUFSIZE;
				pIoData->wsaBuf.buf = pIoData->buffer;
				pIoData->operationType = ACCEPT;
				lpfnAcceptEx(mServerSock, pIoData->sock, exBuf,
					0,
					sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
					&bytes, &(pIoData->overlapped)
				);
				//cout << "������ ���� �� : " << users.size() << endl;
				continue;
			}

			int offset = 0;
			// Network - Job - Event
			while (dwBytesTransferred - offset > 0)
			{
				switch (pIoData->operationType)
				{
				case READ:
				{
					int* packetType = (int*)&pIoData->buffer[offset];
					switch (*packetType)
					{
					case ACTION_PACKET:
					{
						ActionPacket* action = reinterpret_cast<ActionPacket*>(&pIoData->buffer[offset]);
						//cout << "Action Packet Recv" << endl;
						//cout << pHandleData->sock << endl;
						//cout << action->id << endl;
						//cout << action->position[0] << action->position[1] << action->position[2] << endl;
						//cout << action->rotation[0] << action->rotation[1] << action->rotation[2] << endl;
						_userManager->setUserPos(users[pHandleData->sock], (char*)action);
						// �ڽ� �� ���� JobBuf�� �߰�
						for (auto iter = users.begin(); iter != users.end(); iter++) {
							if (pHandleData->sock != iter->first) {
								_userManager->setJob(iter->second, (char*)action);
								jobQueue.push_back(iter->second);
							}
						}
						offset += sizeof(ActionPacket);
						break;
					}

					case EVENT_PACKET:
					{
						// Event�� Y���� �����ؼ� �����ش�.
						EventPacket* eventData = reinterpret_cast<EventPacket*>(&pIoData->buffer[offset]);
						_eventManager->setEvent(eventData);
						offset += sizeof(EventPacket);
						break;
					}
					case USER_VIEW:
						// ������ �ٶ󺸴� �� & ��������
						break;
					}
					break;
				}
				case WRITE:
					offset += pIoData->wsaBuf.len;
					delete pIoData;
					break;
				default: cout << "DEFAULT" << endl;
					dwBytesTransferred = 0;
					break;
				}

				// Job - Action->������ client buffer�� ������ ���� / thread�� ����
				if (!jobQueue.empty()) {
					char* data = NULL;
					data = _userManager->getJob(jobQueue.front());
					if (data != NULL) {
						SOCKET sock;
						for (auto iter = users.begin(); iter != users.end(); iter++) {
							if (iter->second == jobQueue.front()) {
								sock = iter->first;
								break;
							}
						}
						jobQueue.erase(jobQueue.begin());
						int* type = (int*)data;
						LPER_IO_DATA ov = new PER_IO_DATA();
						memset(&(ov->overlapped), 0, sizeof(OVERLAPPED));
						memset(&(ov->buffer), 0, BUFSIZE);
						ov->operationType = WRITE;
						DWORD dwSend = 0;
						if (*type == USER_PACKET || *type == USER_UPDATE_PACKET)
							ov->wsaBuf.len = sizeof(User);
						else if (*type == ACTION_PACKET)
							ov->wsaBuf.len = sizeof(ActionPacket);
						else if (*type == EVENT_PACKET)
							ov->wsaBuf.len = sizeof(EventPacket);
						else if (*type == MONSTER_PACKET)
							ov->wsaBuf.len = sizeof(Monster);
						ov->wsaBuf.buf = data;
						WSASend(sock, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
					}
				}

				//Event
				if (gameStart) {
					time_t toc = clock() - tic;
					if (eventFlag.compare_exchange_weak(eventExpected, false) && !eventFlag) {
						for (int i = 0; i < _eventManager->size(); i++) {
							if (toc >= _eventManager->getEventTime(i)) {
								// �̺�Ʈ ����
								// ������ ���� ��ü���� ����
								// user - overlapped
								switch (i)
								{
								case 0:
									//monster
									break;
								case 1:case 2:case 6:case 7:
								{
									broadcastSend(EVENT_PACKET, i);
									_eventManager->updateEvent(i, clock());
									cout << "Event �߻�>>" << i << endl;
									break;
								}
								case 3:case 4:case 5:
									// 1�ʸ� �ø��ϴ�
									_eventManager->updateEvent(i, clock());
									break;
								case 9:
									_eventManager->updateEvent(i, clock());
								} // random ���� & ����
								  // �ش� �̺�Ʈ�� ������ ������ �ִ°�
								for (auto iter = users.begin(); iter != users.end(); iter++) {
									// userInEvent -> state�� true���� false������ Ȯ���ؼ� return
									if (_eventManager->userInEvent(i, _userManager->getUserPos(iter->second))) {
										// ������ �ش� �̺�Ʈ(i) ��
										// ��¿�
										EventPacket* p = reinterpret_cast<EventPacket*>(_eventManager->getEvent(i));
										cout << "Event ����>>" << endl;
										cout << p->id << endl;
										cout << p->position[0] << p->position[1] << p->position[2] << endl;
										cout << p->state << endl;
										cout << iter->first << endl;;

										if (i == 1 || i == 2) {
											int emo[4] = { 20, 0, 0, 0 };
											_userManager->setUserEmotion(iter->second, emo);
											_eventManager->updateEvent(i, clock());
											broadcastSend(USER_UPDATE_PACKET, iter->second);
										}
										else if (i == 3 || i == 4) {
											int emo[4] = { 0, 3, 0, 0 };
											_userManager->setUserEmotion(iter->second, emo);
											broadcastSend(USER_UPDATE_PACKET, iter->second);
										}
										else if (i == 5) {
											int emo[4] = { 0, 0, 3, 0 };
											_userManager->setUserEmotion(iter->second, emo);
											broadcastSend(USER_UPDATE_PACKET, iter->second);
										}
										else if (i == 6 || i == 7) {
											int emo[4] = { 0, 0, 0, 20 };
											_userManager->setUserEmotion(iter->second, emo);
											_eventManager->updateEvent(i, clock());
											broadcastSend(USER_UPDATE_PACKET, iter->second);
										}
									}// if user in Event
								}// for users
							}// if �̺�Ʈ �߻�
						} // for:��� �̺�Ʈ Ȯ��
						eventFlag = true;
					} // �̺�Ʈ ����
					else
						eventExpected = true;// atomic event
				} // gamestart-event

				//Monster
				tok = clock() - tek;
				if (tok >= 25) {
					if (_monsterManager->getStart()
						&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
						// �̵�-������ queue�� ����
						_monsterManager->upDate();
						char* data = _monsterManager->getJob();
						if (data) {
							// ATK
							if ((*(int*)data) == MONSTER_PACKET_ATK) {
								Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
								_userManager->setUserHp(atk->target, _monsterManager->getDmg());

								cout << "ATK>>" << atk->target << endl;
								//��¿�
								Monster* a = (Monster*)_monsterManager->getMonsterInfo();
								//cout << a->position[0] << " " << a->position[1] << " " << a->position[2] << endl;
								/*
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
								_userManager->setJob(monster_iter->second, _userManager->getUserInfo(atk->target));
								jobQueue.push_back(monster_iter->second);
								}
								*/
								broadcastSend(USER_UPDATE_PACKET, atk->target);
							}
							// �̵�
							else
								/*
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
								_userManager->setJob(monster_iter->second, data);
								jobQueue.push_back(monster_iter->second);
								}
								*/
								broadcastSend(MONSTER_PACKET, 0);
						}
						MonsterFlag = true;
					}
					else
						monsterExpected = true;
					tek = clock();
				}
			}// Network - while

			// ���ο� overlapped ����ü�� �������� �ʰ� �״�� ����
			pIoData = new PER_IO_DATA();
			memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
			memset(&(pIoData->buffer), 0, BUFSIZE);
			pIoData->wsaBuf.len = BUFSIZE;
			pIoData->wsaBuf.buf = pIoData->buffer;
			pIoData->operationType = READ;
			pIoData->sock = pHandleData->sock;
			dwFlag = 0;
			WSARecv(pHandleData->sock, &(pIoData->wsaBuf), 1, NULL, &dwFlag, &(pIoData->overlapped), NULL);
		}

		//timeout
		else {
			if (GetLastError() != WAIT_TIMEOUT) {
				// 0 Byte�ε� Disconnect �Լ� ȣ�� �� - ��
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
							// ��ó�� �ʿ�
							ErrorHandling("Disconnect Error");
						}
					}
					break;
				}
				else if (pIoData->operationType == DISCONNECT) {
					cout << "pIoData->operationType : ACCEPTEX" << endl;
					// ������ ���� �����ϰ� �����Ǿ��� Overlapped�� Disconnect ����
					// ���� ���� �� AcceptEx ���
					// Overlapped �ʱ�ȭ
					//users.erase(pIoData->sock);

					// overlapped �ʱ�ȭ �� 
					memset(&(pIoData->overlapped), 0, sizeof(OVERLAPPED));
					memset(&(pIoData->buffer), 0, sizeof(BUFSIZE));
					pIoData->wsaBuf.len = BUFSIZE;
					pIoData->wsaBuf.buf = pIoData->buffer;
					pIoData->operationType = ACCEPT;
					lpfnAcceptEx(mServerSock, pIoData->sock, exBuf,
						0,
						sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
						&bytes, &(pIoData->overlapped)
					);
					//cout << "������ ���� �� : " << users.size() << endl;
					continue;
				}
			}
			// Job - Action->������ client buffer�� ������ ���� / thread�� ����
			if (!jobQueue.empty()) {
				char* data = NULL;
				data = _userManager->getJob(jobQueue.front());
				if (data != NULL) {
					SOCKET sock;
					for (auto iter = users.begin(); iter != users.end(); iter++) {
						if (iter->second == jobQueue.front()) {
							sock = iter->first;
							break;
						}
					}
					jobQueue.erase(jobQueue.begin());
					int* type = (int*)data;
					LPER_IO_DATA ov = new PER_IO_DATA();
					memset(&(ov->overlapped), 0, sizeof(OVERLAPPED));
					memset(&(ov->buffer), 0, BUFSIZE);
					ov->operationType = WRITE;
					DWORD dwSend = 0;
					if (*type == USER_PACKET || *type == USER_UPDATE_PACKET)
						ov->wsaBuf.len = sizeof(User);
					else if (*type == ACTION_PACKET)
						ov->wsaBuf.len = sizeof(ActionPacket);
					else if (*type == EVENT_PACKET)
						ov->wsaBuf.len = sizeof(EventPacket);
					else if(*type == MONSTER_PACKET)
						ov->wsaBuf.len = sizeof(Monster);
					ov->wsaBuf.buf = data;
					WSASend(sock, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
				}
			}
			
			//Event
			if (gameStart) {
				time_t toc = clock() - tic;
				if (eventFlag.compare_exchange_weak(eventExpected, false) && !eventFlag) {
					for (int i = 0; i < _eventManager->size(); i++) {
						if (toc >= _eventManager->getEventTime(i)) {
							// �̺�Ʈ ����
							// ������ ���� ��ü���� ����
							// user - overlapped
							switch (i)
							{
							case 0:
								cout << "���Ͱ� ������Ʈ�� �����ϱ� ����" << endl;
								_eventManager->updateEvent(i, clock());
								break;
							case 1:case 2:case 6:case 7:
							{
								cout << "Event �߻�>>" << i << endl;
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i, clock());
								break;
							}
							case 3:case 4:case 5:
								// 1�ʸ� �ø��ϴ�
								_eventManager->updateEvent(i, clock());
								break;
							} // random ���� & ����
							  // �ش� �̺�Ʈ�� ������ ������ �ִ°�
							for (auto iter = users.begin(); iter != users.end(); iter++) {
								// userInEvent -> state�� true���� false������ Ȯ���ؼ� return
								if (_eventManager->userInEvent(i, _userManager->getUserPos(iter->second))) {
									// ������ �ش� �̺�Ʈ(i) ��
									// ��¿�
									EventPacket* p = reinterpret_cast<EventPacket*>(_eventManager->getEvent(i));
									cout << "Event ����>>" << endl;
									cout << p->id << endl;
									cout << p->position[0] << p->position[1] << p->position[2] << endl;
									cout << p->state << endl;
									cout << iter->first << endl;;

									if (i == 1 || i == 2) {
										int emo[4] = { 20, 0, 0, 0 };
										_userManager->setUserEmotion(iter->second, emo);
										_eventManager->updateEvent(i, clock());
										broadcastSend(USER_UPDATE_PACKET, iter->second);
									}
									else if (i == 3 || i == 4) {
										int emo[4] = { 0, 3, 0, 0 };
										_userManager->setUserEmotion(iter->second, emo);
										broadcastSend(USER_UPDATE_PACKET, iter->second);
									}
									else if (i == 5) {
										int emo[4] = { 0, 0, 3, 0 };
										_userManager->setUserEmotion(iter->second, emo);
										broadcastSend(USER_UPDATE_PACKET, iter->second);
									}
									else if (i == 6 || i == 7) {
										int emo[4] = { 0, 0, 0, 20 };
										_userManager->setUserEmotion(iter->second, emo);
										_eventManager->updateEvent(i, clock());
										broadcastSend(USER_UPDATE_PACKET, iter->second);
									}
								}// if user in Event
							}// for users
						}// if �̺�Ʈ �߻�
					} // for:��� �̺�Ʈ Ȯ��
					eventFlag = true;
				} // �̺�Ʈ ����
				else
					eventExpected = true;// atomic event
			} // gamestart-event

			//Monster
			tok = clock() - tek;
			if (tok >= 25) {
				if (_monsterManager->getStart()
					&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
					// �̵�-������ queue�� ����
					_monsterManager->upDate();
					char* data = _monsterManager->getJob();
					if (data) {
						// ATK
						if ((*(int*)data) == MONSTER_PACKET_ATK) {
							Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
							_userManager->setUserHp(atk->target, _monsterManager->getDmg());

							cout << "ATK>>"<<atk->target << endl;
							//��¿�
							Monster* a = (Monster*)_monsterManager->getMonsterInfo();
							//cout << a->position[0] << " " << a->position[1] << " " << a->position[2] << endl;
							/*
							for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
							_userManager->setJob(monster_iter->second, _userManager->getUserInfo(atk->target));
							jobQueue.push_back(monster_iter->second);
							}
							*/
							broadcastSend(USER_UPDATE_PACKET, atk->target);
						}
						// �̵�
						else
							/*
							for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
							_userManager->setJob(monster_iter->second, data);
							jobQueue.push_back(monster_iter->second);
							}
							*/
							broadcastSend(MONSTER_PACKET, 0);
					}
					MonsterFlag = true;
				}
				else
					monsterExpected = true;
				tek = clock();
			}

		} // time out
	}//GQCS

	return 0;
}

void ErrorHandling(LPCSTR errorMsg)
{
	fputs(errorMsg, stderr);
	fputc('\n', stderr);
	exit(1);
}

