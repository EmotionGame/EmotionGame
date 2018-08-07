#include "stdafx.h"
#include "NetworkEngine.h"
#include "Camera.h"
#include "HID.h"
#include "Model.h"
#include "Player.h"
#include "Event.h"
#include "QuadTree.h"
#include "ModelManager.h"

ModelManager::ModelManager()
{
}
ModelManager::ModelManager(const ModelManager& other)
{
}
ModelManager::~ModelManager()
{
}

/********** Player : ���� **********/
unsigned int __stdcall ModelManager::InitPlayerModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitPlayerModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitPlayerModelThread()
{
	// WIC Texture Loader���� ������ ���ο��� �ؽ�ó�� �ҷ����� ���� �ȵǴµ�
	// �����ϰ� �Ϸ��� CoInitialize(NULL); ���ָ� �˴ϴ�.
	CoInitialize(NULL);

	Player* player = new Player;

	/***** �� ���� �� <���ؽ�> : ��� *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>("Player", player));
	m_AllModelUMapMutex.unlock();
	/***** �� ���� �� <���ؽ�> : ���� *****/

	// �� �ʱ�ȭ ������ ���� �ε� �ʱ�ȭ ����
	player->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/elin.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));

	/***** �÷��̾� �� �ʱ�ȭ ť <���ؽ�> : ��� *****/
	m_InitPlayerQueueMutex.lock();
	for (int i = 0; i < PLAYER_SIZE; i++)
	{
		m_InitPlayerQueue->push(new Player(*player));
	}
	m_InitPlayerQueueMutex.unlock();
	/***** �÷��̾� �� �ʱ�ȭ ť <���ؽ�> : ���� *****/

	/***** �� �ʱ�ȭ <��������> : ���� *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	player->Initialize(m_device, "Data/KSM/X_Bot/X_Bot", L"Data/KSM/Default/Default_1.dds", 
		XMFLOAT3(0.001f, 0.001f, 0.001f), false, 0); // �� �ʱ�ȭ ����
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** �� �ʱ�ȭ <��������> : ���� *****/

	m_PlayerInitilizedMutex.lock();
	m_PlayerInitilized = true;
	m_PlayerInitilizedMutex.unlock();

	_endthreadex(0);
	return true;
}
unsigned int __stdcall ModelManager::CopyPlayerModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_CopyPlayerModelThread();

	return true;
}
UINT WINAPI ModelManager::_CopyPlayerModelThread()
{
	/***** �� ���� �� <���ؽ�> : ��� *****/
	m_AllModelUMapMutex.lock();
	Player* player = static_cast<Player*>(m_AllModelUMap->at("Player")); // ������ "Player"�� �����ϴ�.
	m_AllModelUMapMutex.unlock();
	/***** �� ���� �� <���ؽ�> : ���� *****/

	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		// �ش� ���� �̹� �� �ʱ�ȭ�� �Ǿ��ִٸ� �ǳʶ�ϴ�.
		if (iter->second->IsInitilized())
			continue;

		*iter->second = *player; // �����մϴ�.
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> : ���� *****/

	_endthreadex(0);
	return true;
}
/********** Player : ���� **********/


/********** Event : ���� **********/
unsigned int __stdcall ModelManager::InitEventModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitEventModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitEventModelThread()
{
	// WIC Texture Loader���� ������ ���ο��� �ؽ�ó�� �ҷ����� ���� �ȵǴµ�
	// �����ϰ� �Ϸ��� CoInitialize(NULL); ���ָ� �˴ϴ�.
	CoInitialize(NULL);
	
	m_InitEventCountMutex.lock();
	unsigned int eventCount = m_InitEventCount;
	m_InitEventCount++;
	m_InitEventCountMutex.unlock();

	Event* event = new Event;

	char str[16];
	sprintf_s(str, "Event_%d", eventCount);

	/***** �� ���� �� <���ؽ�> : ��� *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>(str, event));
	m_AllModelUMapMutex.unlock();
	/***** �� ���� �� <���ؽ�> : ���� *****/

	switch (eventCount)
	{
	case 0: // �����
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-10.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PuppyCat/Cat_Triangulated", L"Data/KSM/PuppyCat/PuppyCat.png", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		break;
	case 1: // ������
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-20.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Dog/Dog", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 2: // ��ں�
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-30.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Bonfire/Bonfire", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(1.0f, 1.0f, 1.0f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 3: // ���� �ڰ�
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-40.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Peddle/Peddle", L"Data/KSM/Peddle/Peddle.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 4: // ������
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-50.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Mage/Mage", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 5: // ����
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-60.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Cloud/Cloud", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 6: // �δ���
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-70.0f, 0.0f, -10.0f));
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PoliceOfficer/PoliceOfficer", L"Data/KSM/PoliceOfficer/PoliceOfficer.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	}

	m_EventInitilizedMutex[eventCount].lock();
	m_EventInitilized[eventCount] = true;
	m_EventInitilizedMutex[eventCount].unlock();

	_endthreadex(0);
	return true;
}
/********** Event : ���� **********/


bool ModelManager::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
#ifdef _DEBUG
	printf("Start >> ModelManager.cpp : Initialize()\n");
#endif
	m_device = pDevice;
	m_hwnd = hwnd;

	// ���ÿ� �ִ� 4�� �����常 ���� �ε��ϵ��� �����ϴ� ��������
	m_InitSemaphore = CreateSemaphore(NULL, 4, 4, NULL);

	m_AllModelUMap = new std::unordered_map<std::string, Model*>;
	if (!m_AllModelUMap)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_InitPlayerQueue = new std::queue<Player*>;", L"Error", MB_OK);
		return false;
	}

	/***** Player : ���� *****/
	m_InitPlayerQueue = new std::queue<Player*>;
	if (!m_InitPlayerQueue)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_InitPlayerQueue = new std::queue<Player*>;", L"Error", MB_OK);
		return false;
	}
	m_PlayerUMap = new std::unordered_map<unsigned int, Player*>;
	if (!m_PlayerUMap)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_PlayerUMap = new std::unordered_map<unsigned int, Player*>;", L"Error", MB_OK);
		return false;
	}

	_beginthreadex(NULL, 0, InitPlayerModelThread, (LPVOID)this, 0, NULL);
	/***** Player : ���� *****/

	/***** Event : ���� *****/
	m_EventUMap = new std::unordered_map<unsigned int, Event*>;
	if (!m_EventUMap)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_EventUMap = new std::unordered_map<unsigned int, Event*>;", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < EVENT_SIZE; i++)
	{
		_beginthreadex(NULL, 0, InitEventModelThread, (LPVOID)this, 0, NULL);
	}
	/***** Event : ���� *****/
#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}

void ModelManager::Shutdown()
{
	if (m_AllModelUMap)
	{
		for (auto iter = m_AllModelUMap->begin(); iter != m_AllModelUMap->end(); iter++)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_AllModelUMap->clear();
		delete m_AllModelUMap;
		m_AllModelUMap = nullptr;
	}

	if (m_PlayerUMap)
	{
		for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_PlayerUMap->clear();
		delete m_PlayerUMap;
		m_PlayerUMap = nullptr;
	}

	if (m_InitPlayerQueue)
	{
		while (!m_InitPlayerQueue->empty())
		{
			if (m_InitPlayerQueue->front())
			{
				delete m_InitPlayerQueue->front();
				m_InitPlayerQueue->pop();
			}
		}
		delete m_InitPlayerQueue;
		m_InitPlayerQueue = nullptr;
	}
}

bool ModelManager::Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
{ 
	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);

		// ���� �ʱ�ȭ�� ���� ���� ���
		if (!iter->second->IsInitilized())
		{
			/***** �÷��̾� �� �ʱ�ȭ <���ؽ�> : ��� *****/
			m_PlayerInitilizedMutex.lock();
			if (m_PlayerInitilized)
			{
				_beginthreadex(NULL, 0, CopyPlayerModelThread, (LPVOID)this, 0, NULL);
			}
			m_PlayerInitilizedMutex.unlock();
			/***** �÷��̾� �� �ʱ�ȭ <���ؽ�> : ���� *****/
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> : ���� *****/

	// �̺�Ʈ �׽�Ʈ
	m_AllModelUMapMutex.lock();
	for (auto iter = m_AllModelUMap->begin(); iter != m_AllModelUMap->end(); iter++)
	{
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);
	}
	m_AllModelUMapMutex.unlock();
	// �̺�Ʈ �׽�Ʈ

	return true;
}

bool ModelManager::Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	// ��Ʈ��ũ ���ῡ �������� ���� �����մϴ�.
	if (pNetworkEngine->GetConnectFlag())
	{
		PlayerPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	}

	return true;
}

int ModelManager::GetPlayerID()
{
	return m_PlayerID;
}

void ModelManager::DetectChangingValue(int playerID)
{
	// ��ġ�� ������ �Ǿ��� ����
	if ((m_PlayerPastPos.x != m_PlayerPresentPos.x) || (m_PlayerPastPos.y != m_PlayerPresentPos.y) || (m_PlayerPastPos.z != m_PlayerPresentPos.z))
	{
		m_PlayerPastPos = m_PlayerPresentPos;
		m_DetectChanging[playerID] = true;
	}

	// ȸ���� ������ �Ǿ��� ����
	if ((m_PlayerPastRot.x != m_PlayerPresentRot.x) || (m_PlayerPastRot.y != m_PlayerPresentRot.y) || (m_PlayerPastRot.z != m_PlayerPresentRot.z))
	{
		m_PlayerPastRot = m_PlayerPresentRot;
		m_DetectChanging[playerID] = true;
	}
}

bool ModelManager::PlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	CreatePlayerPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	
	RecvUserPacket31Physics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	RecvActionPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	PlayerActionPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	
	CreateEventPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	return true;
}

bool ModelManager::CreatePlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ��� *****/
	pNetworkEngine->GetRecvUP30QueueMutex().lock();
	while (!pNetworkEngine->GetRecvUP30Queue()->empty()) // GetRecvUPQueue�� �� ������ ����
	{
		UserPacket player = pNetworkEngine->GetRecvUP30Queue()->front();
		pNetworkEngine->GetRecvUP30QueueMutex().unlock();
		/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ���� *****/

		int id = player.id;
		int type = player.type;

		// Ű�� �Ѱ��� id�� �ش�Ǵ� ���Ұ� ���� �ʱ�ȭ �س��� ���� ���� ������

		/***** �÷��̾� ť <���ؽ�>, �÷��̾� U�� <���ؽ�> : ��� *****/
		m_InitPlayerQueueMutex.lock();
		m_PlayerUMapMutex.lock();

		// m_PlayerUMap�� �ش� id�� key�� ���Ұ� �������� �ʰ� m_PlayerQueue�� ���Ұ� �����Ѵٸ�
		if ((m_PlayerUMap->find(id) == m_PlayerUMap->end()) && !m_InitPlayerQueue->empty())
		{
			m_PlayerUMap->emplace(std::pair<int, Player*>(id, m_InitPlayerQueue->front()));
			m_InitPlayerQueue->pop();
			m_InitPlayerQueueMutex.unlock();
			/***** �÷��̾� ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_InitPlayerQueue->pop();\n");
#endif
			/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ��� *****/
			pNetworkEngine->GetRecvUP30QueueMutex().lock();
			pNetworkEngine->GetRecvUP30Queue()->pop();
			pNetworkEngine->GetRecvUP30QueueMutex().unlock();
			/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ���� *****/

			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));

			// ù ��° ��Ŷ�� �ش� Ŭ���̾�Ʈ�� �÷��̾ �˴ϴ�.
			if (!m_SetPlayerID)
			{
				m_SetPlayerID = true;
				m_PlayerID = id;
			}

			/***** �÷��̾� ť <���ؽ�> : ��� *****/
			m_InitPlayerQueueMutex.lock();
		}
		else
		{
			m_PlayerUMapMutex.unlock();
			m_InitPlayerQueueMutex.unlock();
			/***** �÷��̾� ť <���ؽ�>, �÷��̾� U�� <���ؽ�> : ���� *****/
			/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ��� *****/
			pNetworkEngine->GetRecvUP30QueueMutex().lock();
			break;
		}

		m_PlayerUMapMutex.unlock();
		m_InitPlayerQueueMutex.unlock();
		/***** �÷��̾� ť <���ؽ�>, �÷��̾� U�� <���ؽ�> : ���� *****/

		/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ��� *****/
		pNetworkEngine->GetRecvUP30QueueMutex().lock();
	}
	pNetworkEngine->GetRecvUP30QueueMutex().unlock();
	/***** ������Ŷ �ʱ�ȭ ť <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::RecvUserPacket31Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** ������Ŷ ť <���ؽ�> : ��� *****/
	pNetworkEngine->GetRecvUP31QueueMutex().lock();
	while (!pNetworkEngine->GetRecvUP31Queue()->empty()) // GetRecvUP31Queue�� �� ������ ����
	{
		UserPacket player = pNetworkEngine->GetRecvUP31Queue()->front();
		pNetworkEngine->GetRecvUP31Queue()->pop();
		pNetworkEngine->GetRecvUP31QueueMutex().unlock();
		/***** ������Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : m_RecvUserPacket31Queue->pop();\n");
#endif
		int id = player.id;

		/***** �÷��̾� U�� <���ؽ�> : ��� *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // Ű�� �����ϰ� Ȱ��ȭ �����̸�
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			m_PlayerUMap->at(id)->SetHP(player.hp);
			m_PlayerUMap->at(id)->SetSpeed(player.speed);
			m_PlayerUMap->at(id)->SetEmotion(player.emotion);
		}
		m_PlayerUMapMutex.unlock();
		/***** �÷��̾� U�� <���ؽ�> : ���� *****/

		/***** ������Ŷ ť <���ؽ�> : ��� *****/
		pNetworkEngine->GetRecvUP31QueueMutex().lock();
	}
	pNetworkEngine->GetRecvUP31QueueMutex().unlock();
	/***** ������Ŷ ť <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::RecvActionPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** �׼���Ŷ ť <���ؽ�> : ��� *****/
	pNetworkEngine->GetRecvAPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvAPQueue()->empty()) // GetRecvAPQueue�� �� ������ ����
	{
		ActionPacket player = pNetworkEngine->GetRecvAPQueue()->front();
		pNetworkEngine->GetRecvAPQueue()->pop();
		pNetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** �׼���Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");
#endif
		int id = player.id;

		/***** �÷��̾� U�� <���ؽ�> : ��� *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // Ű�� �����ϰ� Ȱ��ȭ �����̸�
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
		}
		m_PlayerUMapMutex.unlock();
		/***** �÷��̾� U�� <���ؽ�> : ���� *****/

		/***** �׼���Ŷ ť <���ؽ�> : ��� *****/
		pNetworkEngine->GetRecvAPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvAPQueueMutex().unlock();
	/***** �׼���Ŷ ť <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::PlayerActionPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (m_PlayerUMap->at(m_PlayerID)->IsActive()) // ���� Ȱ��ȭ �����̸�
		{
			m_PlayerUMap->at(m_PlayerID)->PlayerControl(pHID, deltaTime);
			m_PlayerPresentPos = m_PlayerUMap->at(m_PlayerID)->GetPosition();
			if (pQuadTree->GetHeightAtPosition(m_PlayerPresentPos.x, m_PlayerPresentPos.z, m_PlayerPresentPos.y))
			{
				// ī�޶� �Ʒ��� �ﰢ���� �ִ� ��� ī�޶� �� �� ������ ��ġ�մϴ�.
				m_PlayerUMap->at(m_PlayerID)->SetPosition(m_PlayerPresentPos);
			}
			m_PlayerPresentRot = m_PlayerUMap->at(m_PlayerID)->GetRotation();
			pCamera->SetPosition(m_PlayerUMap->at(m_PlayerID)->GetCameraPosition());
			pCamera->SetRotation(m_PlayerPresentRot);
			m_PlayerUMapMutex.unlock();
			/***** �÷��̾� U�� <���ؽ�> : ���� *****/

			// ���� ����
			DetectChangingValue(m_PlayerID);

			// ������ �Ǿ��� ����
			if (m_DetectChanging[m_PlayerID])
			{
				ActionPacket player;
				player.id = m_PlayerID;
				player.position[0] = m_PlayerPresentPos.x;
				player.position[1] = m_PlayerPresentPos.y;
				player.position[2] = m_PlayerPresentPos.z;
				player.rotation[0] = m_PlayerPresentRot.x;
				player.rotation[1] = m_PlayerPresentRot.y;
				player.rotation[2] = m_PlayerPresentRot.z;

				/***** �׼���Ŷ ť <���ؽ�> : ��� *****/
				pNetworkEngine->GetSendAPQueueMutex().lock();
				if (pNetworkEngine->GetSendAPQueue()->size() < QUEUE_LIMIT_SIZE)
				{
					pNetworkEngine->GetSendAPQueue()->push(player);
					m_DetectChanging[m_PlayerID] = false;
#ifdef _DEBUG
					printf("PUSH >> ModelManager.cpp : m_SendPacketQueue->push(player);\n");
#endif
				}
				else
				{
#ifdef _DEBUG
					printf("PUSH_LiMIT >> ModelManager.cpp : m_SendPacketQueue->push(player);\n");
#endif
				}
				pNetworkEngine->GetSendAPQueueMutex().unlock();
				/***** �׼���Ŷ ť <���ؽ�> : ���� *****/
			}
			/***** �÷��̾� U�� <���ؽ�> : ��� *****/
			m_PlayerUMapMutex.lock();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::CreateEventPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{

	return true;
}