#include "stdafx.h"
#include "NetworkEngine.h"
#include "Camera.h"
#include "HID.h"
#include "Model.h"
#include "Player.h"
#include "Event.h"
#include "Monster.h"
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
	player->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/elin.png", 
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), OrientedBoundingBox);

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
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(1, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-10.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PuppyCat/Cat_Triangulated", L"Data/KSM/PuppyCat/PuppyCat.png", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 1: // ������
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(2, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-20.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Dog/Dog", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.005f, 0.005f, 0.005f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 2: // ��ں�
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(3, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-30.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Bonfire/Bonfire", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(1.0f, 1.0f, 1.0f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 3: // ���� �ڰ�
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(4, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-40.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Peddle/Peddle", L"Data/KSM/Peddle/Peddle.dds", XMFLOAT3(0.05f, 0.05f, 0.05f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 4: // ������
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(6, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-50.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Mage/Mage", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 5: // ����
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(7, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-60.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Cloud/Cloud", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	case 6: // �δ���
		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(8, event));
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-70.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** �� �ʱ�ȭ <��������> : ���� *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PoliceOfficer/PoliceOfficer", L"Data/KSM/PoliceOfficer/PoliceOfficer.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // �� �ʱ�ȭ ����
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** �� �ʱ�ȭ <��������> : ���� *****/

		break;
	}

	_endthreadex(0);
	return true;
}
/********** Event : ���� **********/

/********** Monster : ���� **********/
unsigned int __stdcall ModelManager::InitMonsterModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitMonsterModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitMonsterModelThread()
{
	// WIC Texture Loader���� ������ ���ο��� �ؽ�ó�� �ҷ����� ���� �ȵǴµ�
	// �����ϰ� �Ϸ��� CoInitialize(NULL); ���ָ� �˴ϴ�.
	CoInitialize(NULL);

	Monster* monster = new Monster;

	/***** �� ���� �� <���ؽ�> : ��� *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>("Monster", monster));
	m_AllModelUMapMutex.unlock();
	/***** �� ���� �� <���ؽ�> : ���� *****/


	/***** ���� <���ؽ�> : ��� *****/
	m_MonsterMutex.lock();
	m_Monster = monster;
	m_MonsterMutex.unlock();
	/***** ���� <���ؽ�> : ���� *****/

	monster->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png",
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(128.0f, 0.0f, 128.0f), OrientedBoundingBox);
	/***** �� �ʱ�ȭ <��������> : ���� *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	monster->Initialize(m_device, "Data/KSM/Cannon/Cannon", L"Data/KSM/Cannon/Cannon.dds", XMFLOAT3(10.0f, 10.0f, 10.0f), false, 0); // �� �ʱ�ȭ ����
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** �� �ʱ�ȭ <��������> : ���� *****/

	_endthreadex(0);
	return true;
}
/********** Monster : ���� **********/

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

	/***** Monster : ���� *****/
	m_Monster = new Monster;
	if (!m_Monster)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_Monster = new Monster;", L"Error", MB_OK);
		return false;
	}
	_beginthreadex(NULL, 0, InitMonsterModelThread, (LPVOID)this, 0, NULL);
	/***** Monster : ���� *****/

#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}

void ModelManager::Shutdown()
{
	if (m_Monster)
	{
		delete m_Monster;
		m_Monster = nullptr;
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

	if (m_AllModelUMap)
	{
		delete m_AllModelUMap->at("Player");

		m_AllModelUMap->clear();
		delete m_AllModelUMap;
		m_AllModelUMap = nullptr;
	}
}

bool ModelManager::Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
{ 
	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		iter->second->PlayerAnimation(deltaTime);
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);

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

	/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
	m_EventUMapMutex.lock();
	for (auto iter = m_EventUMap->begin(); iter != m_EventUMap->end(); iter++)
	{
		// Ȱ��ȭ ������ ���� �������մϴ�.
		if (iter->second->GetActive())
		{
			iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
		}
	}
	m_EventUMapMutex.unlock();
	/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

	/***** ���� <���ؽ�> : ��� *****/
	m_MonsterMutex.lock();
	// Ȱ��ȭ ������ ���� �������մϴ�.
	if (m_Monster->GetActive())
	{
		m_Monster->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
	}
	m_MonsterMutex.unlock();
	/***** ���� <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F2))
	{
		m_LineRenderFlag = m_LineRenderFlag ? false : true;
	}

	// ��Ʈ��ũ ���ῡ �������� ���� �����մϴ�.
	if (pNetworkEngine->GetConnectFlag())
	{
		PlayerPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

		EventPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

		MonsterPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	}

	CollisionDetection(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

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

			float PosY = 0.0f;
			if (pQuadTree->GetHeightAtPosition(player.position[0], player.position[2], PosY))
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], PosY, player.position[2]));
			}
			else
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			}
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
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvUP31Queue()->pop();\n");
#endif
		int id = player.id;

		/***** �÷��̾� U�� <���ؽ�> : ��� *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->GetActive()) // Ű�� �����ϰ� Ȱ��ȭ �����̸�
		{
			float PosY;
			if (pQuadTree->GetHeightAtPosition(player.position[0], player.position[2], PosY))
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], PosY, player.position[2]));
			}
			else
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			}
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			m_PlayerUMap->at(id)->SetHP(player.hp);
			m_PlayerUMap->at(id)->SetSpeed(player.speed);
			m_PlayerUMap->at(id)->SetEmotion(player.emotion);
			m_PlayerUMap->at(id)->SetAcceleration(player.acceleration);
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
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvAPQueue()->pop();\n");
#endif
		int id = player.id;

		/***** �÷��̾� U�� <���ؽ�> : ��� *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->GetActive()) // Ű�� �����ϰ� Ȱ��ȭ �����̸�
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			m_PlayerUMap->at(id)->SetAcceleration(player.acceleration);
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
		if (m_PlayerUMap->at(m_PlayerID)->GetActive()) // ���� Ȱ��ȭ �����̸�
		{
			m_PlayerUMap->at(m_PlayerID)->PlayerControl(pHID, pQuadTree, deltaTime);
			m_PlayerPresentPos = m_PlayerUMap->at(m_PlayerID)->GetPosition();
			m_PlayerPresentRot = m_PlayerUMap->at(m_PlayerID)->GetRotation();
			pCamera->SetPosition(m_PlayerUMap->at(m_PlayerID)->GetCameraPosition());
			pCamera->SetRotation(m_PlayerPresentRot);
			XMFLOAT3 acceleration = m_PlayerUMap->at(m_PlayerID)->GetAcceleration();
			m_PlayerUMapMutex.unlock();
			/***** �÷��̾� U�� <���ؽ�> : ���� *****/

			// ���� ����
			DetectChangingValue(m_PlayerID);

			// ������ �Ǿ��� ����
			if (m_DetectChanging[m_PlayerID])
			{
				m_PlayerActionCheck = clock();
				if ((m_PlayerActionCheck - m_PlayerActionInit) >= 50)
				{
					m_PlayerActionInit = clock();
					ActionPacket player;
					player.id = m_PlayerID;
					player.position[0] = m_PlayerPresentPos.x;
					player.position[1] = m_PlayerPresentPos.y;
					player.position[2] = m_PlayerPresentPos.z;
					player.rotation[0] = m_PlayerPresentRot.x;
					player.rotation[1] = m_PlayerPresentRot.y;
					player.rotation[2] = m_PlayerPresentRot.z;
					player.acceleration[0] = acceleration.x;
					player.acceleration[1] = acceleration.y;
					player.acceleration[2] = acceleration.z;

					/***** �׼���Ŷ ť <���ؽ�> : ��� *****/
					pNetworkEngine->GetSendAPQueueMutex().lock();
					if (pNetworkEngine->GetSendAPQueue()->size() < QUEUE_LIMIT_SIZE)
					{
						pNetworkEngine->GetSendAPQueue()->push(player);
						m_DetectChanging[m_PlayerID] = false;
#ifdef _DEBUG
						printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendAPQueue()->push(player);\n");
#endif
					}
					else
					{
#ifdef _DEBUG
						printf("PUSH_LiMIT >> ModelManager.cpp : pNetworkEngine->GetSendAPQueue()->push(player);\n");
#endif
					}
					pNetworkEngine->GetSendAPQueueMutex().unlock();
					/***** �׼���Ŷ ť <���ؽ�> : ���� *****/
				}
			}
			/***** �÷��̾� U�� <���ؽ�> : ��� *****/
			m_PlayerUMapMutex.lock();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::EventPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	RecvEventPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	SendEventPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	return true;
}

bool ModelManager::RecvEventPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** �̺�Ʈ��Ŷ ť <���ؽ�> : ��� *****/
	pNetworkEngine->GetRecvEPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvEPQueue()->empty()) // GetRecvEPQueue�� �� ������ ����
	{
		EventPacket event = pNetworkEngine->GetRecvEPQueue()->front();
		pNetworkEngine->GetRecvEPQueue()->pop();
		pNetworkEngine->GetRecvEPQueueMutex().unlock();
		/***** �̺�Ʈ��Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvEPQueue()->pop();\n");
#endif
		int id = event.id;

		printf("event >> id = %d, position = (%f, %f, %f), state = %s\n",
			event.id,
			event.position[0],
			event.position[1],
			event.position[2],
			event.state ? "true" : "false"
			);

		/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
		m_EventUMapMutex.lock();
		if ((m_EventUMap->find(id) != m_EventUMap->end())) // Ű�� �����ϸ�
		{
			m_EventUMap->at(id)->SetActive(event.state);

			if (m_EventUMap->at(id)->GetActive())
			{
				float posY;
				if (pQuadTree->GetHeightAtPosition(event.position[0], event.position[2], posY))
				{
					m_EventUMap->at(id)->SetPosition(XMFLOAT3(event.position[0], posY, event.position[2]));
				}
				else
				{
					m_EventUMap->at(id)->SetPosition(XMFLOAT3(event.position[0], event.position[1], event.position[2]));
				}
			}
		}
		m_EventUMapMutex.unlock();
		/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

		/***** �̺�Ʈ��Ŷ ť <���ؽ�> : ��� *****/
		pNetworkEngine->GetRecvEPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvEPQueueMutex().unlock();
	/***** �̺�Ʈ��Ŷ ť <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::SendEventPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{


	return true;
}

bool ModelManager::MonsterPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	RecvMonsterPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	SendMonsterPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	return true;
}

bool ModelManager::RecvMonsterPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** ������Ŷ ť <���ؽ�> : ��� *****/
	pNetworkEngine->GetRecvMPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvMPQueue()->empty()) // GetRecvMPQueue�� �� ������ ����
	{
		MonsterPacket monster = pNetworkEngine->GetRecvMPQueue()->front();
		pNetworkEngine->GetRecvMPQueue()->pop();
		pNetworkEngine->GetRecvMPQueueMutex().unlock();
		/***** ������Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvMPQueue()->pop();\n");
#endif
		/***** ���� <���ؽ�> : ��� *****/
		m_MonsterMutex.lock();
		if (m_Monster->GetActive())
		{
			m_Monster->SetSpeed(monster.speed);
			m_Monster->SetEmotion(monster.emotion);
			float posY;
			if (pQuadTree->GetHeightAtPosition(monster.position[0], monster.position[2], posY))
			{
				m_Monster->SetPosition(XMFLOAT3(monster.position[0], posY, monster.position[2]));
			}
			else
			{
				m_Monster->SetPosition(XMFLOAT3(monster.position[0], monster.position[1], monster.position[2]));
			}
			m_Monster->SetRotation(XMFLOAT3(monster.rotation[0], monster.rotation[1], monster.rotation[2]));
			m_Monster->SetDamage(monster.damage);
		}
		m_MonsterMutex.unlock();
		/***** ���� <���ؽ�> : ���� *****/

		/***** ������Ŷ ť <���ؽ�> : ��� *****/
		pNetworkEngine->GetRecvMPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvMPQueueMutex().unlock();
	/***** ������Ŷ ť <���ؽ�> : ���� *****/

	return true;
}

bool ModelManager::SendMonsterPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{


	return true;
}

bool ModelManager::CollisionDetection(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** �浹 üũ �ʱ�ȭ : ���� *****/
	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	for (auto iterPlayer = m_PlayerUMap->begin(); iterPlayer != m_PlayerUMap->end(); iterPlayer++)
	{
		if (iterPlayer->second->IsInitilized())
		{
			iterPlayer->second->InitCollisionCheck();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> :  ���� *****/

	/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
	m_EventUMapMutex.lock();
	for (auto iterEvent = m_EventUMap->begin(); iterEvent != m_EventUMap->end(); iterEvent++)
	{
		if (iterEvent->second->IsInitilized())
		{
			iterEvent->second->InitCollisionCheck();
		}
	}
	m_EventUMapMutex.unlock();
	/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

	/***** ���� <���ؽ�> : ��� *****/
	m_MonsterMutex.lock();
	if (m_Monster->IsInitilized())
	{
		m_Monster->InitCollisionCheck();
	}
	m_MonsterMutex.unlock();
	/***** ���� <���ؽ�> : ���� *****/
	/***** �浹 üũ �ʱ�ȭ : ���� *****/

	/***** �浹 �˻� : ���� *****/
	/***** �÷��̾� U�� <���ؽ�> : ��� *****/
	m_PlayerUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		for (auto iterPlayer = m_PlayerUMap->begin(); iterPlayer != m_PlayerUMap->end(); iterPlayer++)
		{
			if (iterPlayer->first == m_PlayerID)
				continue;

			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && iterPlayer->second->IsInitilized())
			{
				// Ȱ��ȭ ������ ���� �˻��մϴ�.
				if (iterPlayer->second->GetActive())
				{
					if (m_PlayerUMap->at(m_PlayerID)->Intersection(*static_cast<Model*>(iterPlayer->second)))
					{

					}
				}
			}
		}
	}

	/***** �̺�Ʈ U�� <���ؽ�> : ��� *****/
	m_EventUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		for (auto iterEvent = m_EventUMap->begin(); iterEvent != m_EventUMap->end(); iterEvent++)
		{
			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && iterEvent->second->IsInitilized())
			{
				// Ȱ��ȭ ������ ���� �˻��մϴ�.
				if (iterEvent->second->GetActive())
				{
					if (m_PlayerUMap->at(m_PlayerID)->Intersection(*static_cast<Model*>(iterEvent->second)))
					{
						EventAcquirePacket eap;
						eap.eventId = iterEvent->first;
						eap.playerId = m_PlayerID;
						XMFLOAT3 position = iterEvent->second->GetPosition();
						eap.position[0] = position.x;
						eap.position[1] = position.y;
						eap.position[2] = position.z;

						// ���� �̺�Ʈ
						if (iterEvent->first == 1 || iterEvent->first == 2 || iterEvent->first == 7 || iterEvent->first == 8)
						{
							/***** �̺�Ʈ ȹ�� ��Ŷ ť <���ؽ�> : ��� *****/
							pNetworkEngine->GetSendEAPQueueMutex().lock();
							pNetworkEngine->GetSendEAPQueue()->push(eap);
							pNetworkEngine->GetSendEAPQueueMutex().unlock();
							/***** �̺�Ʈ ȹ�� ��Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
							printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendEAPQueue()->push(eap);\n");
#endif
							iterEvent->second->SetActive(false);
						}
						// ���� �̺�Ʈ
						else if (iterEvent->first == 3 || iterEvent->first == 4 || iterEvent->first == 6)
						{
							m_FixedEventCheck = clock();
							if ((m_FixedEventCheck - m_FixedEventInit) >= 1000)
							{
								m_FixedEventInit = clock();

								/***** �̺�Ʈ ȹ�� ��Ŷ ť <���ؽ�> : ��� *****/
								pNetworkEngine->GetSendEAPQueueMutex().lock();
								pNetworkEngine->GetSendEAPQueue()->push(eap);
								pNetworkEngine->GetSendEAPQueueMutex().unlock();
								/***** �̺�Ʈ ȹ�� ��Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
								printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendEAPQueue()->push(eap);\n");
#endif
							}

						}
					}
				}
			}
		}
	}
	m_EventUMapMutex.unlock();
	/***** �̺�Ʈ U�� <���ؽ�> : ���� *****/

	/***** ���� <���ؽ�> : ��� *****/
	m_MonsterMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && m_Monster->IsInitilized())
		{
			// Ȱ��ȭ ������ ���� �˻��մϴ�.
			if (m_Monster->GetActive())
			{
				if (m_PlayerUMap->at(m_PlayerID)->Intersection(*static_cast<Model*>(m_Monster)))
				{
					m_MonsterCheck = clock();
					if ((m_MonsterCheck - m_MonsterInit) >= 2000)
					{
						m_MonsterInit = clock();

						MonsterAttackPacket map;
						map.playerId = m_PlayerID;
						XMFLOAT3 position = m_PlayerUMap->at(m_PlayerID)->GetPosition();
						map.position[0] = position.x;
						map.position[1] = position.y;
						map.position[2] = position.z;
						map.collision = true;

						/***** ���� ���� ��Ŷ ť <���ؽ�> : ��� *****/
						pNetworkEngine->GetSendMAPQueueMutex().lock();
						pNetworkEngine->GetSendMAPQueue()->push(map);
						pNetworkEngine->GetSendMAPQueueMutex().unlock();
						/***** ���� ���� ��Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
						printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendMAPQueue()->push(map);\n");
#endif
					}

				}
				else
				{
					if (0 < (m_MonsterCheck - m_MonsterInit) && (m_MonsterCheck - m_MonsterInit) < 2000)
					{
						m_MonsterInit = m_MonsterCheck;

						MonsterAttackPacket map;
						map.playerId = m_PlayerID;
						XMFLOAT3 position = m_PlayerUMap->at(m_PlayerID)->GetPosition();
						map.position[0] = position.x;
						map.position[1] = position.y;
						map.position[2] = position.z;
						map.collision = false;

						/***** ���� ���� ��Ŷ ť <���ؽ�> : ��� *****/
						pNetworkEngine->GetSendMAPQueueMutex().lock();
						pNetworkEngine->GetSendMAPQueue()->push(map);
						pNetworkEngine->GetSendMAPQueueMutex().unlock();
						/***** ���� ���� ��Ŷ ť <���ؽ�> : ���� *****/
#ifdef _DEBUG
						printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendMAPQueue()->push(map) false collision;\n");
#endif
					}
				}
			}
		}
	}
	m_MonsterMutex.unlock();
	/***** ���� <���ؽ�> : ���� *****/

	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� <���ؽ�> : ���� *****/
	/***** �浹 �˻� : ���� *****/

	return true;
}