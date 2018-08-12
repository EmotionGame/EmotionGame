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

/********** Player : 시작 **********/
unsigned int __stdcall ModelManager::InitPlayerModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitPlayerModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitPlayerModelThread()
{
	// WIC Texture Loader에서 스레드 내부에서 텍스처를 불러오는 것은 안되는데
	// 가능하게 하려면 CoInitialize(NULL); 해주면 됩니다.
	CoInitialize(NULL);

	Player* player = new Player;

	/***** 모델 저장 맵 <뮤텍스> : 잠금 *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>("Player", player));
	m_AllModelUMapMutex.unlock();
	/***** 모델 저장 맵 <뮤텍스> : 해제 *****/

	// 본 초기화 이전에 지연 로딩 초기화 진행
	player->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/elin.png", 
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), OrientedBoundingBox);

	/***** 플레이어 모델 초기화 큐 <뮤텍스> : 잠금 *****/
	m_InitPlayerQueueMutex.lock();
	for (int i = 0; i < PLAYER_SIZE; i++)
	{
		m_InitPlayerQueue->push(new Player(*player));
	}
	m_InitPlayerQueueMutex.unlock();
	/***** 플레이어 모델 초기화 큐 <뮤텍스> : 해제 *****/

	/***** 모델 초기화 <세마포어> : 진입 *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	player->Initialize(m_device, "Data/KSM/X_Bot/X_Bot", L"Data/KSM/Default/Default_1.dds", 
		XMFLOAT3(0.001f, 0.001f, 0.001f), false, 0); // 본 초기화 진행
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** 모델 초기화 <세마포어> : 퇴장 *****/

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
	/***** 모델 저장 맵 <뮤텍스> : 잠금 *****/
	m_AllModelUMapMutex.lock();
	Player* player = static_cast<Player*>(m_AllModelUMap->at("Player")); // 저장한 "Player"를 꺼냅니다.
	m_AllModelUMapMutex.unlock();
	/***** 모델 저장 맵 <뮤텍스> : 해제 *****/

	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		// 해당 모델이 이미 본 초기화가 되어있다면 건너띕니다.
		if (iter->second->IsInitilized())
			continue;

		*iter->second = *player; // 복사합니다.
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

	_endthreadex(0);
	return true;
}
/********** Player : 종료 **********/


/********** Event : 시작 **********/
unsigned int __stdcall ModelManager::InitEventModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitEventModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitEventModelThread()
{
	// WIC Texture Loader에서 스레드 내부에서 텍스처를 불러오는 것은 안되는데
	// 가능하게 하려면 CoInitialize(NULL); 해주면 됩니다.
	CoInitialize(NULL);
	
	m_InitEventCountMutex.lock();
	unsigned int eventCount = m_InitEventCount;
	m_InitEventCount++;
	m_InitEventCountMutex.unlock();

	Event* event = new Event;

	char str[16];
	sprintf_s(str, "Event_%d", eventCount);

	/***** 모델 저장 맵 <뮤텍스> : 잠금 *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>(str, event));
	m_AllModelUMapMutex.unlock();
	/***** 모델 저장 맵 <뮤텍스> : 해제 *****/

	switch (eventCount)
	{
	case 0: // 고양이
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(1, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-10.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PuppyCat/Cat_Triangulated", L"Data/KSM/PuppyCat/PuppyCat.png", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 1: // 강아지
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(2, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-20.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Dog/Dog", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.005f, 0.005f, 0.005f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 2: // 모닥불
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(3, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-30.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Bonfire/Bonfire", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(1.0f, 1.0f, 1.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 3: // 지압 자갈
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(4, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-40.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Peddle/Peddle", L"Data/KSM/Peddle/Peddle.dds", XMFLOAT3(0.05f, 0.05f, 0.05f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 4: // 모래지옥
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(6, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-50.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Mage/Mage", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 5: // 번개
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(7, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-60.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Cloud/Cloud", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 6: // 두더지
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(8, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-70.0f, 0.0f, -10.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PoliceOfficer/PoliceOfficer", L"Data/KSM/PoliceOfficer/PoliceOfficer.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	}

	_endthreadex(0);
	return true;
}
/********** Event : 종료 **********/

/********** Monster : 시작 **********/
unsigned int __stdcall ModelManager::InitMonsterModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitMonsterModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitMonsterModelThread()
{
	// WIC Texture Loader에서 스레드 내부에서 텍스처를 불러오는 것은 안되는데
	// 가능하게 하려면 CoInitialize(NULL); 해주면 됩니다.
	CoInitialize(NULL);

	Monster* monster = new Monster;

	/***** 모델 저장 맵 <뮤텍스> : 잠금 *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>("Monster", monster));
	m_AllModelUMapMutex.unlock();
	/***** 모델 저장 맵 <뮤텍스> : 해제 *****/


	/***** 몬스터 <뮤텍스> : 잠금 *****/
	m_MonsterMutex.lock();
	m_Monster = monster;
	m_MonsterMutex.unlock();
	/***** 몬스터 <뮤텍스> : 해제 *****/

	monster->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png",
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(128.0f, 0.0f, 128.0f), OrientedBoundingBox);
	/***** 모델 초기화 <세마포어> : 진입 *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	monster->Initialize(m_device, "Data/KSM/Cannon/Cannon", L"Data/KSM/Cannon/Cannon.dds", XMFLOAT3(10.0f, 10.0f, 10.0f), false, 0); // 본 초기화 진행
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** 모델 초기화 <세마포어> : 퇴장 *****/

	_endthreadex(0);
	return true;
}
/********** Monster : 종료 **********/

bool ModelManager::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
#ifdef _DEBUG
	printf("Start >> ModelManager.cpp : Initialize()\n");
#endif
	m_device = pDevice;
	m_hwnd = hwnd;

	// 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어
	m_InitSemaphore = CreateSemaphore(NULL, 4, 4, NULL);

	m_AllModelUMap = new std::unordered_map<std::string, Model*>;
	if (!m_AllModelUMap)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_InitPlayerQueue = new std::queue<Player*>;", L"Error", MB_OK);
		return false;
	}

	/***** Player : 시작 *****/
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
	/***** Player : 종료 *****/

	/***** Event : 시작 *****/
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
	/***** Event : 종료 *****/

	/***** Monster : 시작 *****/
	m_Monster = new Monster;
	if (!m_Monster)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_Monster = new Monster;", L"Error", MB_OK);
		return false;
	}
	_beginthreadex(NULL, 0, InitMonsterModelThread, (LPVOID)this, 0, NULL);
	/***** Monster : 종료 *****/

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
	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		iter->second->PlayerAnimation(deltaTime);
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);

		// 아직 초기화가 되지 않은 경우
		if (!iter->second->IsInitilized())
		{
			/***** 플레이어 본 초기화 <뮤텍스> : 잠금 *****/
			m_PlayerInitilizedMutex.lock();
			if (m_PlayerInitilized)
			{
				_beginthreadex(NULL, 0, CopyPlayerModelThread, (LPVOID)this, 0, NULL);
			}
			m_PlayerInitilizedMutex.unlock();
			/***** 플레이어 본 초기화 <뮤텍스> : 해제 *****/
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

	/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
	m_EventUMapMutex.lock();
	for (auto iter = m_EventUMap->begin(); iter != m_EventUMap->end(); iter++)
	{
		// 활성화 상태일 때만 렌더링합니다.
		if (iter->second->GetActive())
		{
			iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
		}
	}
	m_EventUMapMutex.unlock();
	/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

	/***** 몬스터 <뮤텍스> : 잠금 *****/
	m_MonsterMutex.lock();
	// 활성화 상태일 때만 렌더링합니다.
	if (m_Monster->GetActive())
	{
		m_Monster->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
	}
	m_MonsterMutex.unlock();
	/***** 몬스터 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F2))
	{
		m_LineRenderFlag = m_LineRenderFlag ? false : true;
	}

	// 네트워크 연결에 성공했을 때만 실행합니다.
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
	// 위치가 변경이 되었을 때만
	if ((m_PlayerPastPos.x != m_PlayerPresentPos.x) || (m_PlayerPastPos.y != m_PlayerPresentPos.y) || (m_PlayerPastPos.z != m_PlayerPresentPos.z))
	{
		m_PlayerPastPos = m_PlayerPresentPos;
		m_DetectChanging[playerID] = true;
	}

	// 회전이 변경이 되었을 때만
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
	/***** 유저패킷 초기화 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvUP30QueueMutex().lock();
	while (!pNetworkEngine->GetRecvUP30Queue()->empty()) // GetRecvUPQueue가 빌 때까지 실행
	{
		UserPacket player = pNetworkEngine->GetRecvUP30Queue()->front();
		pNetworkEngine->GetRecvUP30QueueMutex().unlock();
		/***** 유저패킷 초기화 큐 <뮤텍스> : 해제 *****/

		int id = player.id;
		int type = player.type;

		// 키로 넘겨준 id에 해당되는 원소가 없고 초기화 해놓은 모델이 남아 있으면

		/***** 플레이어 큐 <뮤텍스>, 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_InitPlayerQueueMutex.lock();
		m_PlayerUMapMutex.lock();

		// m_PlayerUMap에 해당 id가 key인 원소가 존재하지 않고 m_PlayerQueue에 원소가 존재한다면
		if ((m_PlayerUMap->find(id) == m_PlayerUMap->end()) && !m_InitPlayerQueue->empty())
		{
			m_PlayerUMap->emplace(std::pair<int, Player*>(id, m_InitPlayerQueue->front()));
			m_InitPlayerQueue->pop();
			m_InitPlayerQueueMutex.unlock();
			/***** 플레이어 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_InitPlayerQueue->pop();\n");
#endif
			/***** 유저패킷 초기화 큐 <뮤텍스> : 잠금 *****/
			pNetworkEngine->GetRecvUP30QueueMutex().lock();
			pNetworkEngine->GetRecvUP30Queue()->pop();
			pNetworkEngine->GetRecvUP30QueueMutex().unlock();
			/***** 유저패킷 초기화 큐 <뮤텍스> : 해제 *****/

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

			// 첫 번째 패킷이 해당 클라이언트의 플레이어가 됩니다.
			if (!m_SetPlayerID)
			{
				m_SetPlayerID = true;
				m_PlayerID = id;
			}

			/***** 플레이어 큐 <뮤텍스> : 잠금 *****/
			m_InitPlayerQueueMutex.lock();
		}
		else
		{
			m_PlayerUMapMutex.unlock();
			m_InitPlayerQueueMutex.unlock();
			/***** 플레이어 큐 <뮤텍스>, 플레이어 U맵 <뮤텍스> : 해제 *****/
			/***** 유저패킷 초기화 큐 <뮤텍스> : 잠금 *****/
			pNetworkEngine->GetRecvUP30QueueMutex().lock();
			break;
		}

		m_PlayerUMapMutex.unlock();
		m_InitPlayerQueueMutex.unlock();
		/***** 플레이어 큐 <뮤텍스>, 플레이어 U맵 <뮤텍스> : 해제 *****/

		/***** 유저패킷 초기화 큐 <뮤텍스> : 잠금 *****/
		pNetworkEngine->GetRecvUP30QueueMutex().lock();
	}
	pNetworkEngine->GetRecvUP30QueueMutex().unlock();
	/***** 유저패킷 초기화 큐 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::RecvUserPacket31Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** 유저패킷 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvUP31QueueMutex().lock();
	while (!pNetworkEngine->GetRecvUP31Queue()->empty()) // GetRecvUP31Queue가 빌 때까지 실행
	{
		UserPacket player = pNetworkEngine->GetRecvUP31Queue()->front();
		pNetworkEngine->GetRecvUP31Queue()->pop();
		pNetworkEngine->GetRecvUP31QueueMutex().unlock();
		/***** 유저패킷 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvUP31Queue()->pop();\n");
#endif
		int id = player.id;

		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->GetActive()) // 키가 존재하고 활성화 상태이면
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
		/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

		/***** 유저패킷 큐 <뮤텍스> : 잠금 *****/
		pNetworkEngine->GetRecvUP31QueueMutex().lock();
	}
	pNetworkEngine->GetRecvUP31QueueMutex().unlock();
	/***** 유저패킷 큐 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::RecvActionPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** 액션패킷 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvAPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvAPQueue()->empty()) // GetRecvAPQueue가 빌 때까지 실행
	{
		ActionPacket player = pNetworkEngine->GetRecvAPQueue()->front();
		pNetworkEngine->GetRecvAPQueue()->pop();
		pNetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** 액션패킷 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvAPQueue()->pop();\n");
#endif
		int id = player.id;

		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->GetActive()) // 키가 존재하고 활성화 상태이면
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			m_PlayerUMap->at(id)->SetAcceleration(player.acceleration);
		}
		m_PlayerUMapMutex.unlock();
		/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

		/***** 액션패킷 큐 <뮤텍스> : 잠금 *****/
		pNetworkEngine->GetRecvAPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvAPQueueMutex().unlock();
	/***** 액션패킷 큐 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::PlayerActionPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (m_PlayerUMap->at(m_PlayerID)->GetActive()) // 모델이 활성화 상태이면
		{
			m_PlayerUMap->at(m_PlayerID)->PlayerControl(pHID, pQuadTree, deltaTime);
			m_PlayerPresentPos = m_PlayerUMap->at(m_PlayerID)->GetPosition();
			m_PlayerPresentRot = m_PlayerUMap->at(m_PlayerID)->GetRotation();
			pCamera->SetPosition(m_PlayerUMap->at(m_PlayerID)->GetCameraPosition());
			pCamera->SetRotation(m_PlayerPresentRot);
			XMFLOAT3 acceleration = m_PlayerUMap->at(m_PlayerID)->GetAcceleration();
			m_PlayerUMapMutex.unlock();
			/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

			// 변경 감지
			DetectChangingValue(m_PlayerID);

			// 변경이 되었을 때만
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

					/***** 액션패킷 큐 <뮤텍스> : 잠금 *****/
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
					/***** 액션패킷 큐 <뮤텍스> : 해제 *****/
				}
			}
			/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
			m_PlayerUMapMutex.lock();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

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
	/***** 이벤트패킷 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvEPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvEPQueue()->empty()) // GetRecvEPQueue가 빌 때까지 실행
	{
		EventPacket event = pNetworkEngine->GetRecvEPQueue()->front();
		pNetworkEngine->GetRecvEPQueue()->pop();
		pNetworkEngine->GetRecvEPQueueMutex().unlock();
		/***** 이벤트패킷 큐 <뮤텍스> : 해제 *****/
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

		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		if ((m_EventUMap->find(id) != m_EventUMap->end())) // 키가 존재하면
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
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		/***** 이벤트패킷 큐 <뮤텍스> : 잠금 *****/
		pNetworkEngine->GetRecvEPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvEPQueueMutex().unlock();
	/***** 이벤트패킷 큐 <뮤텍스> : 해제 *****/

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
	/***** 몬스터패킷 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvMPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvMPQueue()->empty()) // GetRecvMPQueue가 빌 때까지 실행
	{
		MonsterPacket monster = pNetworkEngine->GetRecvMPQueue()->front();
		pNetworkEngine->GetRecvMPQueue()->pop();
		pNetworkEngine->GetRecvMPQueueMutex().unlock();
		/***** 몬스터패킷 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
		printf("POP >> ModelManager.cpp : pNetworkEngine->GetRecvMPQueue()->pop();\n");
#endif
		/***** 몬스터 <뮤텍스> : 잠금 *****/
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
		/***** 몬스터 <뮤텍스> : 해제 *****/

		/***** 몬스터패킷 큐 <뮤텍스> : 잠금 *****/
		pNetworkEngine->GetRecvMPQueueMutex().lock();
	}
	pNetworkEngine->GetRecvMPQueueMutex().unlock();
	/***** 몬스터패킷 큐 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::SendMonsterPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{


	return true;
}

bool ModelManager::CollisionDetection(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** 충돌 체크 초기화 : 시작 *****/
	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	for (auto iterPlayer = m_PlayerUMap->begin(); iterPlayer != m_PlayerUMap->end(); iterPlayer++)
	{
		if (iterPlayer->second->IsInitilized())
		{
			iterPlayer->second->InitCollisionCheck();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> :  해제 *****/

	/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
	m_EventUMapMutex.lock();
	for (auto iterEvent = m_EventUMap->begin(); iterEvent != m_EventUMap->end(); iterEvent++)
	{
		if (iterEvent->second->IsInitilized())
		{
			iterEvent->second->InitCollisionCheck();
		}
	}
	m_EventUMapMutex.unlock();
	/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

	/***** 몬스터 <뮤텍스> : 잠금 *****/
	m_MonsterMutex.lock();
	if (m_Monster->IsInitilized())
	{
		m_Monster->InitCollisionCheck();
	}
	m_MonsterMutex.unlock();
	/***** 몬스터 <뮤텍스> : 해제 *****/
	/***** 충돌 체크 초기화 : 종료 *****/

	/***** 충돌 검사 : 시작 *****/
	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		for (auto iterPlayer = m_PlayerUMap->begin(); iterPlayer != m_PlayerUMap->end(); iterPlayer++)
		{
			if (iterPlayer->first == m_PlayerID)
				continue;

			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && iterPlayer->second->IsInitilized())
			{
				// 활성화 상태일 때만 검사합니다.
				if (iterPlayer->second->GetActive())
				{
					if (m_PlayerUMap->at(m_PlayerID)->Intersection(*static_cast<Model*>(iterPlayer->second)))
					{

					}
				}
			}
		}
	}

	/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
	m_EventUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		for (auto iterEvent = m_EventUMap->begin(); iterEvent != m_EventUMap->end(); iterEvent++)
		{
			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && iterEvent->second->IsInitilized())
			{
				// 활성화 상태일 때만 검사합니다.
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

						// 생성 이벤트
						if (iterEvent->first == 1 || iterEvent->first == 2 || iterEvent->first == 7 || iterEvent->first == 8)
						{
							/***** 이벤트 획득 패킷 큐 <뮤텍스> : 잠금 *****/
							pNetworkEngine->GetSendEAPQueueMutex().lock();
							pNetworkEngine->GetSendEAPQueue()->push(eap);
							pNetworkEngine->GetSendEAPQueueMutex().unlock();
							/***** 이벤트 획득 패킷 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
							printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendEAPQueue()->push(eap);\n");
#endif
							iterEvent->second->SetActive(false);
						}
						// 고정 이벤트
						else if (iterEvent->first == 3 || iterEvent->first == 4 || iterEvent->first == 6)
						{
							m_FixedEventCheck = clock();
							if ((m_FixedEventCheck - m_FixedEventInit) >= 1000)
							{
								m_FixedEventInit = clock();

								/***** 이벤트 획득 패킷 큐 <뮤텍스> : 잠금 *****/
								pNetworkEngine->GetSendEAPQueueMutex().lock();
								pNetworkEngine->GetSendEAPQueue()->push(eap);
								pNetworkEngine->GetSendEAPQueueMutex().unlock();
								/***** 이벤트 획득 패킷 큐 <뮤텍스> : 해제 *****/
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
	/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

	/***** 몬스터 <뮤텍스> : 잠금 *****/
	m_MonsterMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && m_Monster->IsInitilized())
		{
			// 활성화 상태일 때만 검사합니다.
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

						/***** 몬스터 공격 패킷 큐 <뮤텍스> : 잠금 *****/
						pNetworkEngine->GetSendMAPQueueMutex().lock();
						pNetworkEngine->GetSendMAPQueue()->push(map);
						pNetworkEngine->GetSendMAPQueueMutex().unlock();
						/***** 몬스터 공격 패킷 큐 <뮤텍스> : 해제 *****/
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

						/***** 몬스터 공격 패킷 큐 <뮤텍스> : 잠금 *****/
						pNetworkEngine->GetSendMAPQueueMutex().lock();
						pNetworkEngine->GetSendMAPQueue()->push(map);
						pNetworkEngine->GetSendMAPQueueMutex().unlock();
						/***** 몬스터 공격 패킷 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
						printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendMAPQueue()->push(map) false collision;\n");
#endif
					}
				}
			}
		}
	}
	m_MonsterMutex.unlock();
	/***** 몬스터 <뮤텍스> : 해제 *****/

	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/
	/***** 충돌 검사 : 종료 *****/

	return true;
}