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
	player->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/elin.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));

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
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-10.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PuppyCat/Cat_Triangulated", L"Data/KSM/PuppyCat/PuppyCat.png", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/
		break;
	case 1: // 강아지
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-20.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Dog/Dog", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 2: // 모닥불
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-30.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Bonfire/Bonfire", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(1.0f, 1.0f, 1.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 3: // 지압 자갈
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-40.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Peddle/Peddle", L"Data/KSM/Peddle/Peddle.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 4: // 모래지옥
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-50.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Mage/Mage", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.1f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 5: // 번개
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-60.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Cloud/Cloud", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 6: // 두더지
		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-70.0f, 0.0f, -10.0f));
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PoliceOfficer/PoliceOfficer", L"Data/KSM/PoliceOfficer/PoliceOfficer.dds", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	}

	m_EventInitilizedMutex[eventCount].lock();
	m_EventInitilized[eventCount] = true;
	m_EventInitilizedMutex[eventCount].unlock();

	_endthreadex(0);
	return true;
}
/********** Event : 종료 **********/


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
	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);

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

	// 이벤트 테스트
	m_AllModelUMapMutex.lock();
	for (auto iter = m_AllModelUMap->begin(); iter != m_AllModelUMap->end(); iter++)
	{
		iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);
	}
	m_AllModelUMapMutex.unlock();
	// 이벤트 테스트

	return true;
}

bool ModelManager::Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	// 네트워크 연결에 성공했을 때만 실행합니다.
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
	
	CreateEventPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

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

			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
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
		printf("POP >> ModelManager.cpp : m_RecvUserPacket31Queue->pop();\n");
#endif
		int id = player.id;

		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // 키가 존재하고 활성화 상태이면
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			m_PlayerUMap->at(id)->SetHP(player.hp);
			m_PlayerUMap->at(id)->SetSpeed(player.speed);
			m_PlayerUMap->at(id)->SetEmotion(player.emotion);
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
		printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");
#endif
		int id = player.id;

		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // 키가 존재하고 활성화 상태이면
		{
			m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
			m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
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
		if (m_PlayerUMap->at(m_PlayerID)->IsActive()) // 모델이 활성화 상태이면
		{
			m_PlayerUMap->at(m_PlayerID)->PlayerControl(pHID, deltaTime);
			m_PlayerPresentPos = m_PlayerUMap->at(m_PlayerID)->GetPosition();
			if (pQuadTree->GetHeightAtPosition(m_PlayerPresentPos.x, m_PlayerPresentPos.z, m_PlayerPresentPos.y))
			{
				// 카메라 아래에 삼각형이 있는 경우 카메라를 두 개 단위로 배치합니다.
				m_PlayerUMap->at(m_PlayerID)->SetPosition(m_PlayerPresentPos);
			}
			m_PlayerPresentRot = m_PlayerUMap->at(m_PlayerID)->GetRotation();
			pCamera->SetPosition(m_PlayerUMap->at(m_PlayerID)->GetCameraPosition());
			pCamera->SetRotation(m_PlayerPresentRot);
			m_PlayerUMapMutex.unlock();
			/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

			// 변경 감지
			DetectChangingValue(m_PlayerID);

			// 변경이 되었을 때만
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

				/***** 액션패킷 큐 <뮤텍스> : 잠금 *****/
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
				/***** 액션패킷 큐 <뮤텍스> : 해제 *****/
			}
			/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
			m_PlayerUMapMutex.lock();
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::CreateEventPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{

	return true;
}