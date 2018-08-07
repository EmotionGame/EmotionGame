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

private:
	HWND m_hwnd;

	HANDLE m_hSendThread = NULL;
	HANDLE m_hRecvThread = NULL;

	char* m_IP;
	char* m_Port;

	WSADATA m_wsaData;		  // Winsock ������ ���� ���� ������ ���� �� �ִ� ����
	SOCKET m_hSocket;		  // Ŭ���̾�Ʈ���� ���� �ڵ�
	SOCKADDR_IN m_ServerAddr; // ������ ���� �ּ� ����ü : �ּ� ������ ��� �ִ� ����ü
	
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
};

/*
socket() �Լ��� ������ ���� :  ���ŷ ���� + ���� �����
ioctlsocket() �Լ� ��� : �ͺ��ŷ ���� + ���� �����
WSASocket() �Լ��� ������ ���� : �ͺ��ŷ ���� + �񵿱� �����

Select ��
: �ݺ� ���� + �ͺ��ŷ ���� + ���� ����� + �񵿱� ����

WSAASyncSelect ��
: �ݺ� ���� + �ͺ��ŷ ���� + ���� ����� + ���� ����

WSAEventSelect ��
: �ݺ� ���� + �ͺ��ŷ ���� + ���� ����� + �񵿱� ����

Overlapped(1) ��
: ���� ���� + �ͺ��ŷ ���� + �񵿱� ����� + �񵿱� ����

Overlapped(2) ��
: ���� ���� + �ͺ��ŷ ���� + �񵿱� ����� + �񵿱� ����

��Overlapped(1)�� (2)�� ������ :
(1) : ����� �ϷḦ �̺�Ʈ(Event)�� ���� �˾Ƴ��ϴ�.
(2) : ����� �ϷḦ �Ϸ� ��ƾ(�ݹ� �Լ�)�� ����ؼ� �˾Ƴ��ϴ�.

IOCP ��
: ���� ���� + �ͺ��ŷ ���� + �񵿱� ����� + �񵿱� ����
: ȿ������ ������ ������� ���� ���� ������ ���� ���̶�� �մϴ�.
*/

//WSAEventSelect(); // �̺�Ʈ ��ü�� ����ؼ� ������� �ٷ�ϴ�.
//WSAAsyncSelet(); // ������ �޽��� ���·� ��Ʈ��ũ �̺�Ʈ�� ó���մϴ�.

/* WSAEventSelect �帧
1. WSACreateEvent(:4200)�Լ��� �̺�Ʈ ��ü�� �����Ѵ�.
2. WSAEventSelect(:4200)�Լ��� ���ϰ� �̺�Ʈ�� ¦ ���´�.
3. WSAWaitForMultipleEvents(:4100)�Լ��� �̺�Ʈ�� ��ٸ���.
4. �̺�Ʈ�� �߻��ϸ�, �̺�Ʈ�� ������ Ȯ���ؼ� ó���Ѵ�.
5. �� �̻� ������� �ʴ� �̺�Ʈ�� WSACloseEvent(:4200)�Լ��� �̺�Ʈ ��ü�� �����Ѵ�.

<WSAEventSelect�Լ� 3��° �μ��� �Ѱ��� �̺�Ʈ ���>
FD_READ			�����Ͱ� ���ŵǾ��� ��
FD_WRITE		������ ������ �غ�Ǿ��� ��
FD_OOB			OOB(out of band)�����Ͱ� ���� �Ǿ��� �� 
FD_ACCEPT		���� �䱸�� ������ ��
FD_CONNECT		�����̳�, multi-point join �۾��� �Ϸ�Ǿ��� ��
FD_CLOSE		������ ������ ������ ��
FD_QOS			QOS�� ����Ǿ��ٴ� �޽����� �޾��� ��
FD_GROUP_QOS	�׷� QOS�� ����Ǿ��ٴ� �޽����� �޾��� ��
FD_ROUTING_INTERFACE_CHANGE		������ �������� ���ؼ� ��� ���� �������̽��� ����Ǿ��� ��
FD_ADDRESS_LIST_CHANGE			������ �������� ���տ� ���� ���� ��巹�� ����Ʈ�� ����Ǿ��� ��

WSAEventSelect() �Լ� ��� �� ������ ���� ������ ����.
WSAEventSelect() �Լ��� ȣ���ϸ� �ش� ������ �ڵ����� �ͺ��ŷ ���� ��ȯ�ȴ�.
socket()�Լ��� �����ϴ� ������ ���� ��� ����(ListeningSocket)�� ������ �Ӽ��� ���ϰ� �ȴ�.
���� ��� ������ ���� ������ �ۼ����� ���� �����Ƿ� FD_READ, FD_WRITE �̺�Ʈ�� ó������ �ʴ´�.
�ݸ� accept() �Լ��� �����ϴ� ������ FD_READ, FD_WRITE �̺�Ʈ�� ó���ؾ� �ϹǷ� �ٽ� WSAEventSelect()�Լ��� ȣ���ؼ� �Ӽ��� �����ؾ� �Ѵ�.
WSAEWOULDBLOCK ���� �ڵ带 üũ�ؾ� �Ѵ�.
��Ʈ��ũ �̺�Ʈ �߻� �� ������ ���� �Լ��� ȣ������ ������, ���� ������ ���� ��Ʈ��ũ �̺�Ʈ�� �߻����� �ʴ´�.
���� ��Ʈ��ũ �̺�Ʈ�� �߻��ϸ� ���� �Լ��� ȣ���ؾ� �ϸ�, �׷��� ���� ��� ���ø����̼��� ��Ʈ��ũ �̺�Ʈ �߻� ����� ����صΰ� ���߿� ���� �Լ��� ȣ���ؾ� �Ѵ�.
*��Ʈ��ũ �̺�Ʈ ���� �Լ�
FD_ACCEPT: accept()
FD_READ: recv(), recvfrom()
FD_WRITE: send(), sendto()
FD_CLOSE: ����
FD_CONNECT: ����
FD_OOB: recv(), recvfrom()
*/