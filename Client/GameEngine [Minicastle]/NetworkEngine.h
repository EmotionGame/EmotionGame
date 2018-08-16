#pragma once

struct Packet
{
	int type;
	char buffer[252];
};

class NetworkEngine : public AlignedAllocationPolicy<16>
{
public:
	NetworkEngine();
	~NetworkEngine();

	bool Initialize(HWND hwnd, char* pIP, char* pPort);
	void Shutdown();
	bool Frame();
	bool Connect();

private:
	unsigned int static __stdcall RecvThread(void* p);
	UINT WINAPI _RecvThread();
	unsigned int static __stdcall SendThread(void* p);
	UINT WINAPI _SendThread();

public:
	bool GetConnectFlag();

	std::queue<UserPacket>* GetSendUP30Queue();
	std::queue<UserPacket>* GetRecvUP30Queue();
	std::mutex& GetSendUP30QueueMutex();
	std::mutex& GetRecvUP30QueueMutex();

	std::queue<UserPacket>* GetSendUP31Queue();
	std::queue<UserPacket>* GetRecvUP31Queue();
	std::mutex& GetSendUP31QueueMutex();
	std::mutex& GetRecvUP31QueueMutex();

	std::queue<ActionPacket>* GetSendAPQueue();
	std::queue<ActionPacket>* GetRecvAPQueue();
	std::mutex& GetSendAPQueueMutex();
	std::mutex& GetRecvAPQueueMutex();

	std::queue<EventPacket>* GetSendEPQueue();
	std::queue<EventPacket>* GetRecvEPQueue();
	std::mutex& GetSendEPQueueMutex();
	std::mutex& GetRecvEPQueueMutex();

	std::queue<EventAcquirePacket>* GetSendEAPQueue();
	std::mutex& GetSendEAPQueueMutex();

	std::queue<MonsterPacket>* GetSendMPQueue();
	std::queue<MonsterPacket>* GetRecvMPQueue();
	std::mutex& GetSendMPQueueMutex();
	std::mutex& GetRecvMPQueueMutex();

	std::queue<MonsterAttackPacket>* GetSendMAPQueue();
	std::mutex& GetSendMAPQueueMutex();

	std::queue<ObejctPacket>* GetSendOPQueue();
	std::queue<ObejctPacket>* GetRecvOPQueue();
	std::mutex& GetSendOPQueueMutex();
	std::mutex& GetRecvOPQueueMutex();

	std::queue<Player2Player>* GetSendP2PQueue();
	std::mutex& GetSendP2PQueueMutex();

	std::queue<Player2Monster>* GetSendP2MQueue();
	std::mutex& GetSendP2MQueueMutex();

	std::queue<Player2Object>* GetSendP2OQueue();
	std::mutex& GetSendP2OQueueMutex();

	std::queue<GameOverPacket>* GetRecvGOPQueue();
	std::mutex& GetRecvGOPQueueMutex();

private:
	HWND m_hwnd;

	HANDLE m_hSendThread = NULL;
	HANDLE m_hRecvThread = NULL;

	char* m_IP;
	char* m_Port;

	WSADATA m_wsaData;		  // Winsock 구현에 대한 세부 사항을 얻을 수 있는 변수
	SOCKET m_hSocket;		  // 클라이언트만의 소켓 핸들
	SOCKADDR_IN m_ServerAddr; // 서버의 소켓 주소 구조체 : 주소 정보를 담고 있는 구조체
	
	WSAEVENT m_Event;
	WSAOVERLAPPED m_Overlapped;

	WSABUF m_ConnectBuff;
	WSABUF m_SendBuff;
	int m_SendBytes = 0;
	WSABUF m_RecvBuff;
	int m_RecvBytes = 0;
	int m_RecvFlag = 0;

	bool m_ConnectFlag = false;

	std::queue<UserPacket>* m_SendUserPacket30Queue;
	std::queue<UserPacket>* m_RecvUserPacket30Queue;
	std::mutex m_SendUP30QueueMutex;
	std::mutex m_RecvUP30QueueMutex;

	std::queue<UserPacket>* m_SendUserPacket31Queue;
	std::queue<UserPacket>* m_RecvUserPacket31Queue;
	std::mutex m_SendUP31QueueMutex;
	std::mutex m_RecvUP31QueueMutex;

	std::queue<ActionPacket>* m_SendActionPacketQueue;
	std::queue<ActionPacket>* m_RecvActionPacketQueue;
	std::mutex m_SendAPQueueMutex;
	std::mutex m_RecvAPQueueMutex;

	std::queue<EventPacket>* m_SendEventPacketQueue;
	std::queue<EventPacket>* m_RecvEventPacketQueue;
	std::mutex m_SendEPQueueMutex;
	std::mutex m_RecvEPQueueMutex;

	std::queue<EventAcquirePacket>* m_SendEventAcquirePacketQueue;
	std::mutex m_SendEAPQueueMutex;

	std::queue<MonsterPacket>* m_SendMonsterPacketQueue;
	std::queue<MonsterPacket>* m_RecvMonsterPacketQueue;
	std::mutex m_SendMPQueueMutex;
	std::mutex m_RecvMPQueueMutex;

	std::queue<MonsterAttackPacket>* m_SendMonsterAttackPacketQueue;
	std::mutex m_SendMAPQueueMutex;

	std::queue<ObejctPacket>* m_SendObjectPacketQueue;
	std::queue<ObejctPacket>* m_RecvObjectPacketQueue;
	std::mutex m_SendOPQueueMutex;
	std::mutex m_RecvOPQueueMutex;

	std::queue<Player2Player>* m_SendPlayer2PlayerQueue;
	std::mutex m_SendP2PQueueMutex;

	std::queue<Player2Monster>* m_SendPlayer2MonsterQueue;
	std::mutex m_SendP2MQueueMutex;

	std::queue<Player2Object>* m_SendPlayer2ObjectQueue;
	std::mutex m_SendP2OQueueMutex;

	std::queue<GameOverPacket>* m_RecvGameOverPacketQueue;
	std::mutex m_RecvGOPQueueMutex;
};

/*
socket() 함수로 생성한 소켓 :  블로킹 소켓 + 동기 입출력
ioctlsocket() 함수 사용 : 넌블로킹 소켓 + 동기 입출력
WSASocket() 함수로 생성한 소켓 : 넌블로킹 소켓 + 비동기 입출력

Select 모델
: 반복 서버 + 넌블로킹 소켓 + 동기 입출력 + 비동기 통지

WSAASyncSelect 모델
: 반복 서버 + 넌블로킹 소켓 + 동기 입출력 + 동기 통지

WSAEventSelect 모델
: 반복 서버 + 넌블로킹 소켓 + 동기 입출력 + 비동기 통지

Overlapped(1) 모델
: 병행 서버 + 넌블로킹 소켓 + 비동기 입출력 + 비동기 통지

Overlapped(2) 모델
: 병행 서버 + 넌블로킹 소켓 + 비동기 입출력 + 비동기 통지

※Overlapped(1)과 (2)의 차이점 :
(1) : 입출력 완료를 이벤트(Event)를 통해 알아냅니다.
(2) : 입출력 완료를 완료 루틴(콜백 함수)을 사용해서 알아냅니다.

IOCP 모델
: 병행 서버 + 넌블로킹 소켓 + 비동기 입출력 + 비동기 통지
: 효율적인 스레드 사용으로 가장 좋은 성능을 내는 모델이라고 합니다.
*/

//WSAEventSelect(); // 이벤트 객체를 사용해서 입출력을 다룹니다.
//WSAAsyncSelet(); // 윈도우 메시지 형태로 네트워크 이벤트를 처리합니다.

/* WSAEventSelect 흐름
1. WSACreateEvent(:4200)함수로 이벤트 객체를 생성한다.
2. WSAEventSelect(:4200)함수로 소켓과 이벤트를 짝 짓는다.
3. WSAWaitForMultipleEvents(:4100)함수로 이벤트를 기다린다.
4. 이벤트가 발생하면, 이벤트의 종류를 확인해서 처리한다.
5. 더 이상 사용하지 않는 이벤트는 WSACloseEvent(:4200)함수로 이벤트 객체를 제거한다.

<WSAEventSelect함수 3번째 인수로 넘겨줄 이벤트 목록>
FD_READ			데이터가 수신되었을 때
FD_WRITE		데이터 전송이 준비되었을 때
FD_OOB			OOB(out of band)데이터가 수신 되었을 때 
FD_ACCEPT		접속 요구가 들어왔을 때
FD_CONNECT		접속이나, multi-point join 작업이 완료되었을 때
FD_CLOSE		상대방이 연결을 끊었을 때
FD_QOS			QOS가 변경되었다는 메시지를 받았을 때
FD_GROUP_QOS	그룹 QOS가 변경되었다는 메시지를 받았을 때
FD_ROUTING_INTERFACE_CHANGE		지정된 목적지에 대해서 경로 배정 인터페이스가 변경되었을 때
FD_ADDRESS_LIST_CHANGE			소켓의 프로토콜 집합에 대한 로컬 어드레스 리스트가 변경되었을 때

WSAEventSelect() 함수 사용 시 유의할 점은 다음과 같다.
WSAEventSelect() 함수를 호출하면 해당 소켓은 자동으로 넌블로킹 모드로 전환된다.
socket()함수가 리턴하는 소켓은 연결 대기 소켓(ListeningSocket)과 동일한 속성을 지니게 된다.
연결 대기 소켓은 직접 데이터 송수신을 하지 않으므로 FD_READ, FD_WRITE 이벤트를 처리하지 않는다.
반면 accept() 함수가 리턴하는 소켓은 FD_READ, FD_WRITE 이벤트를 처리해야 하므로 다시 WSAEventSelect()함수를 호출해서 속성을 변경해야 한다.
WSAEWOULDBLOCK 오류 코드를 체크해야 한다.
네트워크 이벤트 발생 시 적절한 소켓 함수를 호출하지 않으면, 다음 번에는 같은 네트워크 이벤트가 발생하지 않는다.
따라서 네트워크 이벤트가 발생하면 대응 함수를 호출해야 하며, 그렇지 않을 경우 애플리케이션이 네트워크 이벤트 발생 사실을 기록해두고 나중에 대응 함수를 호출해야 한다.
*네트워크 이벤트 대응 함수
FD_ACCEPT: accept()
FD_READ: recv(), recvfrom()
FD_WRITE: send(), sendto()
FD_CLOSE: 없음
FD_CONNECT: 없음
FD_OOB: recv(), recvfrom()
*/