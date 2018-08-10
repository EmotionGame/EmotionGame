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
	// 마지막 인자는 Completion Port에 대한 Thread의 수로 프로세스 하나당 하나의 Thread를 두는것이 Context Switching을 피하는 방법이다
	// 0 - CPU 프로세스 하나당 하나의 Thread 생성
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

// __stdcall : wdinwos api에서 사용하고 함수에서 스택을 관리하는 기법
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
			// Data 수신
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

				// 미포함 : GetAddress
				// AcceptEx 함수 종료 - 접속 된 socket을 CP에 등록
				CreateIoCompletionPort((HANDLE)pHandleData->sock, mCompletionPort, (DWORD)pHandleData, 0);

				// 접속한 유저의 정보를 UserManager를 통해 저장
				int id = _userManager->enterUser(pHandleData->sock);
				users.insert({ pHandleData->sock, id });
				char* data = _userManager->getUserInfo(id);
				if (!data)
					ErrorHandling("getUserInfo Error");
				userInfo.push_back((User*)data);

				// 자신에게 접속한 유저 정보 전달 & 기존 접속자에게 자신의 정보 전달
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
				// 초기 접속 시 바로 자신의 아이디 전달
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
				// Thread 종료
				continue;
			}

			// 0 Byte인데 Disconnect 함수 호출 전 - 후
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
				//users.erase(pIoData->sock);
				//_userManager->exitUser(pIoData->sock, users[pIoData->sock]);

				// overlapped 초기화 및 
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
				//cout << "접속한 유저 수 : " << users.size() << endl;
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
						// 자신 외 유저 JobBuf에 추가
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
						// Event의 Y값을 변경해서 돌려준다.
						EventPacket* eventData = reinterpret_cast<EventPacket*>(&pIoData->buffer[offset]);
						_eventManager->setEvent(eventData);
						offset += sizeof(EventPacket);
						break;
					}
					case USER_VIEW:
						// 유저가 바라보는 것 & 감정상태
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

				// Job - Action->각각의 client buffer에 데이터 저장 / thread에 저장
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
								// 이벤트 실행
								// 랜덤은 유저 전체에게 전달
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
									cout << "Event 발생>>" << i << endl;
									break;
								}
								case 3:case 4:case 5:
									// 1초만 올립니다
									_eventManager->updateEvent(i, clock());
									break;
								case 9:
									_eventManager->updateEvent(i, clock());
								} // random 생성 & 삭제
								  // 해당 이벤트에 유저가 접근해 있는가
								for (auto iter = users.begin(); iter != users.end(); iter++) {
									// userInEvent -> state가 true인지 false인지도 확인해서 return
									if (_eventManager->userInEvent(i, _userManager->getUserPos(iter->second))) {
										// 유저가 해당 이벤트(i) 내
										// 출력용
										EventPacket* p = reinterpret_cast<EventPacket*>(_eventManager->getEvent(i));
										cout << "Event 습득>>" << endl;
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
							}// if 이벤트 발생
						} // for:모든 이벤트 확인
						eventFlag = true;
					} // 이벤트 접근
					else
						eventExpected = true;// atomic event
				} // gamestart-event

				//Monster
				tok = clock() - tek;
				if (tok >= 25) {
					if (_monsterManager->getStart()
						&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
						// 이동-공격을 queue에 저장
						_monsterManager->upDate();
						char* data = _monsterManager->getJob();
						if (data) {
							// ATK
							if ((*(int*)data) == MONSTER_PACKET_ATK) {
								Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
								_userManager->setUserHp(atk->target, _monsterManager->getDmg());

								cout << "ATK>>" << atk->target << endl;
								//출력용
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
							// 이동
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

			// 새로운 overlapped 구조체를 생성하지 않고 그대로 재사용
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
				// 0 Byte인데 Disconnect 함수 호출 전 - 후
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
					//users.erase(pIoData->sock);

					// overlapped 초기화 및 
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
					//cout << "접속한 유저 수 : " << users.size() << endl;
					continue;
				}
			}
			// Job - Action->각각의 client buffer에 데이터 저장 / thread에 저장
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
							// 이벤트 실행
							// 랜덤은 유저 전체에게 전달
							// user - overlapped
							switch (i)
							{
							case 0:
								cout << "몬스터가 오브젝트를 공격하기 시작" << endl;
								_eventManager->updateEvent(i, clock());
								break;
							case 1:case 2:case 6:case 7:
							{
								cout << "Event 발생>>" << i << endl;
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i, clock());
								break;
							}
							case 3:case 4:case 5:
								// 1초만 올립니다
								_eventManager->updateEvent(i, clock());
								break;
							} // random 생성 & 삭제
							  // 해당 이벤트에 유저가 접근해 있는가
							for (auto iter = users.begin(); iter != users.end(); iter++) {
								// userInEvent -> state가 true인지 false인지도 확인해서 return
								if (_eventManager->userInEvent(i, _userManager->getUserPos(iter->second))) {
									// 유저가 해당 이벤트(i) 내
									// 출력용
									EventPacket* p = reinterpret_cast<EventPacket*>(_eventManager->getEvent(i));
									cout << "Event 습득>>" << endl;
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
						}// if 이벤트 발생
					} // for:모든 이벤트 확인
					eventFlag = true;
				} // 이벤트 접근
				else
					eventExpected = true;// atomic event
			} // gamestart-event

			//Monster
			tok = clock() - tek;
			if (tok >= 25) {
				if (_monsterManager->getStart()
					&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
					// 이동-공격을 queue에 저장
					_monsterManager->upDate();
					char* data = _monsterManager->getJob();
					if (data) {
						// ATK
						if ((*(int*)data) == MONSTER_PACKET_ATK) {
							Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
							_userManager->setUserHp(atk->target, _monsterManager->getDmg());

							cout << "ATK>>"<<atk->target << endl;
							//출력용
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
						// 이동
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

