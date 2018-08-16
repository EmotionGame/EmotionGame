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
}

IOCPServer::~IOCPServer()
{
	closesocket(mServerSock);
	WSACleanup();
}

void IOCPServer::cleanUp()
{
	// PQCS
}
void IOCPServer::gameEnd()
{
	delete _object;
	delete _monsterManager;
	delete _eventManager;
	delete _userManager;
	return;
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
	DWORD bytes;
	GUID GuidDisconnect = WSAID_DISCONNECTEX;
	LPFN_DISCONNECTEX lpfnDisconnectEx = NULL;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;

	vector<int> jobQueue;

	bool eventExpected = true;
	bool monsterExpected = true;
	bool oneSecExpected = true;
	bool userOutExpected = true;

	while (TRUE)
	{
		timeout = GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred,
			(LPDWORD)&pHandleData, (LPOVERLAPPED*)&pIoData, 1);
		if (timeout) {
			// Data 수신
			if (pHandleData == NULL) {
				// // cout << "pHandleData = NULL";
			}
			// CK - FLAG
			switch ((int)pHandleData)
			{
			case ACCEPT:
			{
				// // cout << "pIoData->operationType : ACCEPT" << endl;
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
				// // cout << "----------------------CURRENT USER COUNT  " << count << "   ----------------------" << endl;
				if (count == USERSIZE) {
					gameStart = true;
					tic = clock();
					if (!gameSetting) {
						for (int i = 3; i <= 5; i++)
							broadcastSend(EVENT_PACKET, i);
						gameSetting = true;
					}
					//_userManager->start();
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
			if (dwBytesTransferred == 0 && pIoData->operationType == DISCONNECT) {
				// // cout << "pIoData->operationType : ACCEPTEX" << endl;
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
				count--;
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
						// // cout << "ACTION PACKET>>"<<action->id << endl;
						// // cout << action->position[0] << action->position[1] << action->position[2] << endl;
						//// // cout << action->rotation[0] << action->rotation[1] << action->rotation[2] << endl;
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
					case USER_EVENT_PACKET:
					{
						// 유저가 이벤트를 먹음
						// // cout << "USER_EVENT_PACKET>>";
						UserEventPacket* getEvent = reinterpret_cast<UserEventPacket*>(&pIoData->buffer[offset]);
						// // cout << getEvent->eventId << endl;
						if (_eventManager->validate(getEvent->eventId, getEvent->pos, true)) {
							if (getEvent->eventId == 1 || getEvent->eventId == 2 || getEvent->eventId == 7 || getEvent->eventId == 8) {
								// // cout << "RAMDOM EVENT PACKET" << endl;
								_eventManager->updateEvent(getEvent->eventId, clock());
								int index = getEvent->eventId;
								if (index > 5)
									index--;
								broadcastSend(EVENT_PACKET, index);
							}
							int* emotion = _eventManager->getEmotion(getEvent->eventId);
							// 이벤트에 맞는 유저&감정 갱신
							_userManager->setUserEmotion(users[getEvent->userId], emotion);

							for (auto iter = users.begin(); iter != users.end(); iter++) {
								_userManager->setJob(iter->second, _userManager->getUserInfo(users[getEvent->userId]));
								jobQueue.push_back(iter->second);
							}
						}
						offset += sizeof(UserEventPacket);
						break;
					}
					case USER_MONSTER_PACKET:
					{
						// // cout << "USER_MONSTER_PACKET" << endl;
						UserMonsterPacket* collisionMonster = reinterpret_cast<UserMonsterPacket*>(&pIoData->buffer[offset]);
						// // cout << "CollisionMonster>>PlayerID>>" << collisionMonster->playerId << endl;
						_monsterManager->validate(users[collisionMonster->playerId], collisionMonster->position, collisionMonster->collision);
						offset += sizeof(UserMonsterPacket);
						break;
					}

					case USER_OBJECT_PACKET:
					{
						// 유저가 오브젝트와 충돌
						// // cout << "USER_OBJECT_PACKET" << endl;
						UserObjectPacket* collisionObject = reinterpret_cast<UserObjectPacket*>(&pIoData->buffer[offset]);
						if (_object->validation(collisionObject->objectId, collisionObject->position, collisionObject->collision)) {
							int objectEvent = _object->effect(collisionObject->objectId);
							if (objectEvent == 1) {
								_userManager->setUserHp(users[collisionObject->playerId], -3);
								_userManager->setJob(users[collisionObject->playerId], _userManager->getUserInfo(users[collisionObject->playerId]));
								jobQueue.push_back(users[collisionObject->playerId]);
							}
							else if (objectEvent == 2) {
								char* data = NULL;
								//_userManager->setUserHp(users[collisionObject->playerId], -3);
								_userManager->setJob(users[collisionObject->playerId], _userManager->getUserInfo(users[collisionObject->playerId]));
								jobQueue.push_back(users[collisionObject->playerId]);
							}
						}
						offset += sizeof(UserObjectPacket);
					}
					case VIEW_PLAYER:
					{
						// // cout << "VIEW_PLAYER>>" << endl;
						Player2Player* pTplayer= reinterpret_cast<Player2Player*>(&pIoData->buffer[offset]);
						_userManager->setUserEmotion(pTplayer->player2Id, pTplayer->emotion);
						int decrease[4] = { 0, 0, 0, 0 };
						for (int i = 0; i < 4; i++)
							if (pTplayer->emotion[i] != 0)
								decrease[i] = pTplayer->emotion[i] * -1;
						_userManager->setUserEmotion(pTplayer->player1Id, decrease);
						
						_userManager->setJob(users[pTplayer->player1Id], _userManager->getUserInfo(users[pTplayer->player1Id]));
						jobQueue.push_back(users[pTplayer->player1Id]);
						_userManager->setJob(users[pTplayer->player2Id], _userManager->getUserInfo(users[pTplayer->player2Id]));
						jobQueue.push_back(users[pTplayer->player2Id]);


						offset += sizeof(Player2Player);
						break;
					}
					case VIEW_MONSTER:
					{
						// // cout << "VIEW_MONSTER>>" << endl;
						Player2Monster* pTmonster = reinterpret_cast<Player2Monster*>(&pIoData->buffer[offset]);
						_monsterManager->setEmotion(pTmonster->emotion);
						offset += sizeof(Player2Monster);
						break;
					}
					case VIEW_OBJECT:
					{
						// // cout << "VIEW_OBJECT>>" << endl;
						Player2Object* pTobject = reinterpret_cast<Player2Object*>(&pIoData->buffer[offset]);
						_object->setEmotion(pTobject->objectId, pTobject->emotion);
						offset += sizeof(Player2Object);
						break;
					}
					}
					break;
				}
				case WRITE:
					offset += pIoData->wsaBuf.len;
					delete pIoData;
					break;
				default: // // cout << "DEFAULT" << endl;
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
						// // cout << "JOBQUEUE>>SEND>>" << sock << endl;
						WSASend(sock, &ov->wsaBuf, 1, &dwSend, 0, &ov->overlapped, NULL);
					}
				}

				//Event
				if (gameStart && eventFlag.compare_exchange_weak(eventExpected, false) && !eventFlag) {
					for (int i = 0; i < _eventManager->size(); i++) {
						if (clock() - tic >= _eventManager->getEventTime(i)) {
							// 이벤트 실행
							// 랜덤은 유저 전체에게 전달
							switch (i)
							{
							case 0:
								//monster
								break;
							case 1:case 2:
							{
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i, clock());
								break;
							}
							case 6:case 7:
							{
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i + 1, clock());
								break;
							}
							case 3:case 4:
								_eventManager->updateEvent(i, clock());
								break;
							case 5:
								// 1초만 올립니다
								_eventManager->updateEvent(i + 1, clock());
								break;
							case 9:
								_eventManager->updateEvent(i, clock());
							default:
								break;
							}
						}// if 이벤트 발생
					} // for:모든 이벤트 확인
					eventFlag = true;
				}
				else
					eventExpected = true;// atomic event

				//Monster
				if (clock() - tek >= 25 && gameStart) {
					if (_monsterManager->getStart()
						&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
						// 이동-공격을 queue에 저장
						_monsterManager->upDate();
						char* data = _monsterManager->getJob();
						if (data) {
							// ATK
							Monster* a = (Monster*)_monsterManager->getMonsterInfo();
							// cout << a->position[0] << " " << a->position[1] << " " << a->position[2] << endl;
							if ((*(int*)data) == MONSTER_PACKET_ATK) {
								Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
								_userManager->setUserHp(atk->target, _monsterManager->getDmg());
								// cout << "ATK>>" << atk->target << endl;
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
									_userManager->setJob(monster_iter->second, _userManager->getUserInfo(atk->target));
									jobQueue.push_back(monster_iter->second);
								}
								//broadcastSend(USER_UPDATE_PACKET, atk->target);
							}
							// 이동
							else
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
									_userManager->setJob(monster_iter->second, data);
									jobQueue.push_back(monster_iter->second);
								}
								//broadcastSend(MONSTER_PACKET, 0);
						}//몬스터 데이터가 없으면
						MonsterFlag = true;
					}
					else
						monsterExpected = true;
					tek = clock();
				}

				// 1 sec event
				if (gameStart && clock() - tok >= 1000 &&
					secFlag.compare_exchange_weak(oneSecExpected, false) && !secFlag) {
					// 모든 오브젝트들의 감정을 -1 && 이를 유저 전체에게 send? // 몬스터는 update를 통해 알아서
					_userManager->update();
					if (!gameover) {
						for (int i = 0; i < users.size(); i += 2) {
							if (!_userManager->alive(i) && !_userManager->alive(i + 1)) {
								broadcastSend(GAMEOVER, i / 2);
								gameover = true;
								gameEnd();
								break;
							}
						}
					}

					secFlag = true;
					tok = clock();
				}
				else
					oneSecExpected = true;
					
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

		//timeout && 종료
		else {
			if (GetLastError() != WAIT_TIMEOUT) {
				// 0 Byte인데 Disconnect 함수 호출 전 - 후
				if (userOutFlag.compare_exchange_weak(userOutExpected, false) && !userOutFlag) {
					if (dwBytesTransferred == 0 && pIoData->operationType != DISCONNECT) {
						//_userManager->exitUser(users[pHandleData->sock]);
						//users[pHandleData->sock] = -1;
						// cout << "TIMEOUT>>0BYTE>>Disconnect" << endl;
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
								// cout << "TIMEOUT>>DISCONNECTEX error" << endl;
								ErrorHandling("Disconnect Error");
							}
						}
					}
					userOutFlag = true;
				}//flag
				else
					userOutExpected = true;
				continue;
			}
			else {
				// TIME OUT
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
				if (gameStart && eventFlag.compare_exchange_weak(eventExpected, false) && !eventFlag) {
					for (int i = 0; i < _eventManager->size(); i++) {
						if (clock() - tic >= _eventManager->getEventTime(i)) {
							// 이벤트 실행
							// 랜덤은 유저 전체에게 전달
							// cout << "TIMEOUT>>EVENT 발생>>" << i << endl;
							switch (i)
							{
							case 0:
								//monster
								break;
							case 1:case 2:
							{
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i, clock());
								break;
							}
							case 6:case 7:
							{
								// cout << i << endl;
								broadcastSend(EVENT_PACKET, i);
								_eventManager->updateEvent(i + 1, clock());
								break;
							}
							case 3:case 4:
								_eventManager->updateEvent(i, clock());
								break;
							case 5:
								// 1초만 올립니다
								_eventManager->updateEvent(i + 1, clock());
								break;
							case 9:
								_eventManager->updateEvent(i, clock());
							default:
								break;
							}
						}// if 이벤트 발생
					} // for:모든 이벤트 확인
					eventFlag = true;
				}
				else
					eventExpected = true;// atomic event

				//Monster
				if (clock() - tek >= 25 && gameStart) {
					if (_monsterManager->getStart()
						&& MonsterFlag.compare_exchange_weak(monsterExpected, false) && !MonsterFlag) {
						// 이동-공격을 queue에 저장
						_monsterManager->upDate();
						char* data = _monsterManager->getJob();
						if (data) {
							// ATK
							Monster* a = (Monster*)_monsterManager->getMonsterInfo();
							// cout << a->position[0] << " " << a->position[1] << " " << a->position[2] << endl;
							if ((*(int*)data) == MONSTER_PACKET_ATK) {
								Monster_ATK* atk = reinterpret_cast<Monster_ATK*>(data);
								_userManager->setUserHp(atk->target, _monsterManager->getDmg());
								// cout << "ATK>>" << atk->target << endl;
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
									_userManager->setJob(monster_iter->second, _userManager->getUserInfo(atk->target));
									jobQueue.push_back(monster_iter->second);
								}
								//broadcastSend(USER_UPDATE_PACKET, atk->target);
							}
							// 이동
							else
								for (auto monster_iter = users.begin(); monster_iter != users.end(); monster_iter++) {
									_userManager->setJob(monster_iter->second, data);
									jobQueue.push_back(monster_iter->second);
								}
							//broadcastSend(MONSTER_PACKET, 0);
						}//몬스터 데이터가 없으면
						MonsterFlag = true;
					}
					else
						monsterExpected = true;
					tek = clock();
				}

				// 1 sec event
				if (gameStart && clock() - tok >= 1000 &&
					secFlag.compare_exchange_weak(oneSecExpected, false) && !secFlag) {
					// 모든 오브젝트들의 감정을 -1 && 이를 유저 전체에게 send? // 몬스터는 update를 통해 알아서
					_userManager->update();
					if (!gameover) {
						for (int i = 0; i < users.size(); i += 2) {
							if (!_userManager->alive(i) && !_userManager->alive(i + 1)) {
								broadcastSend(GAMEOVER, i / 2);
								gameover = true;
								gameEnd();
								break;
							}
						}
					}

					secFlag = true;
					tok = clock();
				}
				else
					oneSecExpected = true;
			}
		}
	}//GQCS

	return 0;
}

void ErrorHandling(LPCSTR errorMsg)
{
	fputs(errorMsg, stderr);
	fputc('\n', stderr);
	exit(1);
}

