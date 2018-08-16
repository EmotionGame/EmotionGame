#include "stdafx.h"
#include "Direct3D.h"
#include "NetworkEngine.h"
#include "Camera.h"
#include "HID.h"
#include "Model.h"
#include "Player.h"
#include "Event.h"
#include "Monster.h"
#include "Object.h"
#include "QuadTree.h"
#include "Emotion.h"
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
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(30.0f, -10.0f, 30.0f), OrientedBoundingBox);

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
	player->Initialize(m_device, "Data/KSM/X_Bot/X_Bot", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.001f, 0.001f, 0.001f), false, 0); // 본 초기화 진행
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/PuppyCat/Cat", L"Data/KSM/PuppyCat/PuppyCat.png", XMFLOAT3(0.01f, 0.01f, 0.01f), false, 0); // 본 초기화 진행
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Sand/Sand", L"Data/KSM/Sand/Sand.jpg", XMFLOAT3(0.0075f, 0.0075f, 0.0075f), false, 0); // 본 초기화 진행
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
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Cloud/Cloud", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.05f, 0.05f, 0.05f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	case 6: // 뱀
		/***** 이벤트 U맵 <뮤텍스> : 잠금 *****/
		m_EventUMapMutex.lock();
		m_EventUMap->emplace(std::pair<unsigned int, Event*>(8, event));
		m_EventUMapMutex.unlock();
		/***** 이벤트 U맵 <뮤텍스> : 해제 *****/

		event->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png", 
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-100.0f, -100.0f, -100.0f), AxisAlignedBoundingBox);
		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		event->Initialize(m_device, "Data/KSM/Snake/Snake", L"Data/KSM/Snake/Snake.png", XMFLOAT3(0.2f, 0.1f, 0.1f), false, 0); // 본 초기화 진행
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

	monster->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/monster.png",
		XMFLOAT3(6.0f, 3.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(128.0f, 0.0f, 128.0f), OrientedBoundingBox);
	/***** 모델 초기화 <세마포어> : 진입 *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	monster->Initialize(m_device, "Data/KSM/Monster/Monster", L"Data/KSM/Monster/Monster.jpg", XMFLOAT3(0.3f, 0.3f, 0.3f), false, 0); // 본 초기화 진행
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** 모델 초기화 <세마포어> : 퇴장 *****/

	_endthreadex(0);
	return true;
}
/********** Monster : 종료 **********/

/********** Object : 시작 **********/
unsigned int __stdcall ModelManager::InitObjectModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitObjectModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitObjectModelThread()
{
	// WIC Texture Loader에서 스레드 내부에서 텍스처를 불러오는 것은 안되는데
	// 가능하게 하려면 CoInitialize(NULL); 해주면 됩니다.
	CoInitialize(NULL);

	m_InitObjectCountMutex.lock();
	unsigned int objectCount = m_InitObjectCount;
	m_InitObjectCount++;
	m_InitObjectCountMutex.unlock();

	Object* object = new Object;

	char str[16];
	sprintf_s(str, "Object_%d", objectCount);

	/***** 모델 저장 맵 <뮤텍스> : 잠금 *****/
	m_AllModelUMapMutex.lock();
	m_AllModelUMap->emplace(std::pair<std::string, Model*>(str, object));
	m_AllModelUMapMutex.unlock();
	/***** 모델 저장 맵 <뮤텍스> : 해제 *****/

	switch (objectCount)
	{
	case 0: // 울타리
		/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
		m_ObjectUMapMutex.lock();
		m_ObjectUMap->emplace(std::pair<unsigned int, Object*>(1, object));
		m_ObjectUMapMutex.unlock();
		/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/

		object->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(10.0f, -10.0f, 30.0f), AxisAlignedBoundingBox);

		for (int i = 2; i <= 4; i++)
		{
			/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
			m_ObjectUMapMutex.lock();
			m_ObjectUMap->emplace(std::pair<unsigned int, Object*>(i, new Object(*object)));
			m_ObjectUMapMutex.unlock();
			/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/
		}

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		object->Initialize(m_device, "Data/KSM/Fence3/Fence3", L"Data/KSM/Fence3/Fence3.dds", XMFLOAT3(0.3f, 0.3f, 0.3f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		for (int i = 2; i <= 4; i++)
		{
			/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
			m_ObjectUMapMutex.lock();
			*m_ObjectUMap->at(i) = *object;
			m_ObjectUMapMutex.unlock();
			/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/
		}

		break;
	case 1: // 움막
		/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
		m_ObjectUMapMutex.lock();
		m_ObjectUMap->emplace(std::pair<unsigned int, Object*>(5, object));
		m_ObjectUMapMutex.unlock();
		/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/

		object->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/popori.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(30.0f, -10.0f, 30.0f), AxisAlignedBoundingBox);


		/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
		m_ObjectUMapMutex.lock();
		m_ObjectUMap->emplace(std::pair<unsigned int, Object*>(6, new Object(*object)));
		m_ObjectUMapMutex.unlock();
		/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/
		

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		object->Initialize(m_device, "Data/KSM/Hut/Hut", L"Data/KSM/Hut/Hut.png", XMFLOAT3(1.0f, 1.0f, 1.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
		m_ObjectUMapMutex.lock();
		*m_ObjectUMap->at(6) = *object;
		m_ObjectUMapMutex.unlock();
		/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/
		
		break;
	}

	_endthreadex(0);
	return true;
}
/********** Object : 종료 **********/

/********** Model : 시작 **********/
unsigned int __stdcall ModelManager::InitModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitModelThread()
{
	// WIC Texture Loader에서 스레드 내부에서 텍스처를 불러오는 것은 안되는데
	// 가능하게 하려면 CoInitialize(NULL); 해주면 됩니다.
	CoInitialize(NULL);

	m_InitModelCountMutex.lock();
	unsigned int modelCount = m_InitModelCount;
	m_InitModelCount++;
	m_InitModelCountMutex.unlock();

	Model* model = new Model;

	switch (modelCount)
	{
	case 0: // 배럴
		m_ModelVecMutex.lock();
		m_ModelVec->push_back(model);
		m_ModelVecMutex.unlock();

		model->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/model.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), AxisAlignedBoundingBox);

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		model->Initialize(m_device, "Data/KSM/Barrel/Barrel", L"Data/KSM/Barrel/Barrel.dds", XMFLOAT3(10.0f, 10.0f, 10.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;

	case 1: // 바구니
		m_ModelVecMutex.lock();
		m_ModelVec->push_back(model);
		m_ModelVecMutex.unlock();

		model->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/model.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(246.0f, 10.0f, 10.0f), AxisAlignedBoundingBox);

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		model->Initialize(m_device, "Data/KSM/Basket/Basket", L"Data/KSM/Basket/Basket.dds", XMFLOAT3(10.0f, 10.0f, 10.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;

	case 2: // 차
		m_ModelVecMutex.lock();
		m_ModelVec->push_back(model);
		m_ModelVecMutex.unlock();

		model->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/model.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 90.0f, 0.0f), XMFLOAT3(10.0f, 10.0f, 246.0f), AxisAlignedBoundingBox);

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		model->Initialize(m_device, "Data/KSM/Car/Car", L"Data/KSM/Car/Car.jpg", XMFLOAT3(2.0f, 2.0f, 2.0f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;

	case 3: // 해적 상자
		m_ModelVecMutex.lock();
		m_ModelVec->push_back(model);
		m_ModelVecMutex.unlock();

		model->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/model.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 45.0f, 0.0f), XMFLOAT3(246.0f, 10.0f, 246.0f), OrientedBoundingBox);

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		model->Initialize(m_device, "Data/KSM/PiratesChest/PiratesChest", L"Data/KSM/PiratesChest/PiratesChest.dds", XMFLOAT3(0.05f, 0.05f, 0.05f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;

	case 4: // 섬
		m_ModelVecMutex.lock();
		m_ModelVec->push_back(model);
		m_ModelVecMutex.unlock();

		model->DelayLoadingInitialize(m_device, m_hwnd, L"Data/DelayLoading/model.png",
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 90.0f, 0.0f), XMFLOAT3(-50.0f, 50.0f, -50.0f), OrientedBoundingBox);

		/***** 모델 초기화 <세마포어> : 진입 *****/
		WaitForSingleObject(m_InitSemaphore, INFINITE);
		model->Initialize(m_device, "Data/KSM/LowPolyMill/LowPolyMill", L"Data/KSM/Default/Default_1.dds", XMFLOAT3(0.5f, 0.5f, 0.5f), false, 0); // 본 초기화 진행
		ReleaseSemaphore(m_InitSemaphore, 1, NULL);
		/***** 모델 초기화 <세마포어> : 퇴장 *****/

		break;
	}

	_endthreadex(0);
	return true;
}
/********** Model : 종료 **********/

bool ModelManager::Initialize(ID3D11Device* pDevice, HWND hwnd, int screenWidth, int screenHeight)
{
#ifdef _DEBUG
	printf("Start >> ModelManager.cpp : Initialize()\n");
#endif
	m_device = pDevice;
	m_hwnd = hwnd;

	m_Emotion = new Emotion;
	if (!m_Emotion)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_Emotion = new Emotion;", L"Error", MB_OK);
		return false;
	}
	if (!m_Emotion->Initialize(pDevice, hwnd, screenWidth, screenHeight, L"Data/DefaultTexture/Transparent.png", screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_Emotion->Initialize()", L"Error", MB_OK);
		return false;
	}

	m_GameWin = new Emotion;
	if (!m_GameWin)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_GameWin = new Emotion;", L"Error", MB_OK);
		return false;
	}
	if (!m_GameWin->Initialize(pDevice, hwnd, screenWidth, screenHeight, L"Data/GameEnd/GameWin.png", screenWidth + 10, screenHeight + 10))
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_GameWin->Initialize()", L"Error", MB_OK);
		return false;
	}

	m_GameOver = new Emotion;
	if (!m_GameOver)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_GameOver = new Emotion;", L"Error", MB_OK);
		return false;
	}
	if (!m_GameOver->Initialize(pDevice, hwnd, screenWidth, screenHeight, L"Data/GameEnd/GameOver.png", screenWidth + 10, screenHeight + 10))
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_GameOver->Initialize()", L"Error", MB_OK);
		return false;
	}

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

	/***** Object : 시작 *****/
	m_ObjectUMap = new std::unordered_map<unsigned int, Object*>;
	if (!m_ObjectUMap)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_ObjectUMap = new std::unordered_map<unsigned int, Object*>;", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < 2; i++)
	{
		_beginthreadex(NULL, 0, InitObjectModelThread, (LPVOID)this, 0, NULL);
	}
	/***** Object : 종료 *****/

	/***** Model : 시작 *****/
	m_ModelVec = new std::vector<Model*>;
	if (!m_ModelVec)
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_ModelVec = new std::vector<Model*>;", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < MODEL_SIZE; i++)
	{
		_beginthreadex(NULL, 0, InitModelThread, (LPVOID)this, 0, NULL);
	}
	/***** Model : 종료 *****/

#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}

void ModelManager::Shutdown()
{
	if (m_ObjectUMap)
	{
		for (auto iter = m_ObjectUMap->begin(); iter != m_ObjectUMap->end(); iter++)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_ObjectUMap->clear();
		delete m_ObjectUMap;
		m_ObjectUMap = nullptr;
	}

	if (m_Monster)
	{
		delete m_Monster;
		m_Monster = nullptr;
	}

	if (m_EventUMap)
	{
		for (auto iter = m_EventUMap->begin(); iter != m_EventUMap->end(); iter++)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_EventUMap->clear();
		delete m_EventUMap;
		m_EventUMap = nullptr;
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

	if (m_Emotion)
	{
		m_Emotion->Shutdown();
		delete m_Emotion;
		m_Emotion = nullptr;
	}

	if (m_GameOver)
	{
		m_GameOver->Shutdown();
		delete m_GameOver;
		m_GameOver = nullptr;
	}

	if (m_GameWin)
	{
		m_GameWin->Shutdown();
		delete m_GameWin;
		m_GameWin = nullptr;
	}
}

bool ModelManager::Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, 
	XMMATRIX baseViewMatrix, XMMATRIX orthoMatrix, 
	XMFLOAT3 cameraPosition, float deltaTime, int screenWidth, int screenHeight)
{ 
	if (!m_GameWinFlag && !m_GameOverFlag)
	{
		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
		{
			if (iter->second->GetActive())
			{
				iter->second->PlayerAnimation(deltaTime);
				iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
			}

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
			m_Monster->PlayerAnimation(deltaTime);
			m_Monster->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
		}
		m_MonsterMutex.unlock();
		/***** 몬스터 <뮤텍스> : 해제 *****/

		/***** 오브젝트 U맵 <뮤텍스> : 잠금 *****/
		m_ObjectUMapMutex.lock();
		for (auto iter = m_ObjectUMap->begin(); iter != m_ObjectUMap->end(); iter++)
		{
			// 활성화 상태일 때만 렌더링합니다.
			if (iter->second->GetActive())
			{
				iter->second->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
			}
		}
		m_ObjectUMapMutex.unlock();
		/***** 오브젝트 U맵 <뮤텍스> : 해제 *****/

		/***** 모델 벡터 <뮤텍스> : 잠금 *****/
		m_ModelVecMutex.lock();
		for (auto iter = m_ModelVec->begin(); iter != m_ModelVec->end(); iter++)
		{
			// 활성화 상태일 때만 렌더링합니다.
			if ((*iter)->GetActive())
			{
				(*iter)->Render(pDirect3D, pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime, m_LineRenderFlag);
			}
		}
		m_ModelVecMutex.unlock();
		/***** 모델 벡터 <뮤텍스> : 해제 *****/

		int max = 0;
		int emotion[4] = { 0, 0, 0, 0 };


		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
		{
			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && m_PlayerUMap->at(m_PlayerID)->GetActive())
			{
				emotion[0] = m_PlayerUMap->at(m_PlayerID)->GetEmotion0();
				emotion[1] = m_PlayerUMap->at(m_PlayerID)->GetEmotion1();
				emotion[2] = m_PlayerUMap->at(m_PlayerID)->GetEmotion2();
				emotion[3] = m_PlayerUMap->at(m_PlayerID)->GetEmotion3();
			}
		}

		for (int i = 1; i <= 3; i++)
		{
			if (emotion[max] < emotion[i])
			{
				max = i;
			}
		}

		m_PlayerUMapMutex.unlock();
		/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

		m_Emotion->Render(pDirect3D, pDeviceContext, baseViewMatrix, orthoMatrix, 0, 0, emotion[max], max, screenWidth, screenHeight);
	}
	else if (m_GameWinFlag)
	{
		m_GameWin->Render(pDirect3D, pDeviceContext, baseViewMatrix, orthoMatrix, -5, -5, 0, 5, screenWidth, screenHeight);
	}
	else if (m_GameOverFlag)
	{
		m_GameOver->Render(pDirect3D, pDeviceContext, baseViewMatrix, orthoMatrix, -5, -5, 0, 5, screenWidth, screenHeight);
	}

	return true;
}

bool ModelManager::Physics(Direct3D* pDirect3D, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime, int screenWidth, int screenHeight)
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

	EmotionSystem(pDirect3D, pHID, pNetworkEngine, pCamera, screenWidth, screenHeight);

	return true;
}

int ModelManager::GetPlayerID()
{
	return m_PlayerID;
}
int ModelManager::GetPlayerHP()
{
	return m_PlayerHP;
}
float ModelManager::GetPlayerSpeed()
{
	return m_PlayerSpeed;
}
int* ModelManager::GetPlayerEmotion()
{
	return m_PlayerEmotion;
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
	if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F3))
	{
		m_PlayerUMapMutex.lock();
		if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
		{
			if (m_PlayerUMap->at(m_PlayerID)->GetActive())
			{
				m_PlayerUMap->at(m_PlayerID)->m_FPS = m_PlayerUMap->at(m_PlayerID)->m_FPS ? false : true;
			}
		}
		m_PlayerUMapMutex.unlock();
	}

	CreatePlayerPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	RecvUserPacket31Physics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	RecvActionPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	PlayerActionPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);
	CheckPlayerActive(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	RecvGameOverPacketPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	ModelPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

	return true;
}

bool ModelManager::CreatePlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	/***** 유저패킷 초기화 큐 <뮤텍스> : 잠금 *****/
	pNetworkEngine->GetRecvUP30QueueMutex().lock();
	while (!pNetworkEngine->GetRecvUP30Queue()->empty()) // GetRecvUPQueue가 빌 때까지 실행
	{
		UserPacket player = pNetworkEngine->GetRecvUP30Queue()->front();
		pNetworkEngine->GetRecvUP30Queue()->pop();
		pNetworkEngine->GetRecvUP30QueueMutex().unlock();
		/***** 유저패킷 초기화 큐 <뮤텍스> : 해제 *****/

		int id = player.id;
		int type = player.type;

		// 키로 넘겨준 id에 해당되는 원소가 없고 초기화 해놓은 모델이 남아 있으면

		/***** 플레이어 큐 <뮤텍스>, 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_InitPlayerQueueMutex.lock();
		m_PlayerUMapMutex.lock();

		// m_PlayerUMap에 해당 id가 key인 원소가 존재하지 않고 m_PlayerQueue에 원소가 존재한다면
		if ((m_PlayerUMap->find(id) == m_PlayerUMap->end()))
		{
			if (!m_InitPlayerQueue->empty())
			{
				m_PlayerUMap->emplace(std::pair<int, Player*>(id, m_InitPlayerQueue->front()));
				m_InitPlayerQueue->pop();
				m_InitPlayerQueueMutex.unlock();
				/***** 플레이어 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
				printf("POP >> ModelManager.cpp : m_InitPlayerQueue->pop();\n");
#endif
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

				if (id == m_PlayerID)
				{
					for (int i = 0; i < 4; i++)
					{
						if (player.emotion[i] < 0 || player.emotion[i] > 100)
						{
							MessageBox(m_hwnd, L"Invalid Emotion Value! 3", L"Error", MB_OK);
						}
					}


					m_PlayerEmotion[0] = player.emotion[0];
					m_PlayerEmotion[1] = player.emotion[1];
					m_PlayerEmotion[2] = player.emotion[2];
					m_PlayerEmotion[3] = player.emotion[3];
				}

				/***** 플레이어 큐 <뮤텍스> : 잠금 *****/
				m_InitPlayerQueueMutex.lock();
			}
		}
		else
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
			m_PlayerUMap->at(id)->SetEmotion0(player.emotion[0]);
			m_PlayerUMap->at(id)->SetEmotion1(player.emotion[1]);
			m_PlayerUMap->at(id)->SetEmotion2(player.emotion[2]);
			m_PlayerUMap->at(id)->SetEmotion3(player.emotion[3]);
			printf("scale = (%f, %f, %f)\n", player.scale[0], player.scale[1], player.scale[2]);
			//m_PlayerUMap->at(id)->SetScale(XMFLOAT3(player.scale[0] * 0.001f , player.scale[1] * 0.001f, player.scale[2] * 0.001f));

			m_PlayerUMap->at(id)->SetAcceleration(player.acceleration);

			if (id == m_PlayerID)
			{
				for (int i = 0; i < 4; i++)
				{
					if (player.emotion[i] < 0 || player.emotion[i] > 100)
					{
						MessageBox(m_hwnd, L"Invalid Emotion Value! 4", L"Error", MB_OK);
					}
				}
				

				m_PlayerEmotion[0] = player.emotion[0];
				m_PlayerEmotion[1] = player.emotion[1];
				m_PlayerEmotion[2] = player.emotion[2];
				m_PlayerEmotion[3] = player.emotion[3];

				m_PlayerHP = player.hp;

				m_PlayerSpeed = player.speed;
			}
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

		m_PlayerHP = player.hp;
		m_PlayerSpeed = player.speed;

		if (id == m_PlayerID)
		{
			m_PlayerEmotion[0] = player.emotion[0];
			m_PlayerEmotion[1] = player.emotion[1];
			m_PlayerEmotion[2] = player.emotion[2];
			m_PlayerEmotion[3] = player.emotion[3];
		}

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
			m_PlayerUMap->at(id)->SetEmotion0(player.emotion[0]);
			m_PlayerUMap->at(id)->SetEmotion1(player.emotion[1]);
			m_PlayerUMap->at(id)->SetEmotion2(player.emotion[2]);
			m_PlayerUMap->at(id)->SetEmotion3(player.emotion[3]);
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
bool ModelManager::CheckPlayerActive(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	m_PlayerUMapMutex.lock();

	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		if (iter->second->IsInitilized())
		{
			if (iter->second->GetActive())
			{
				if (iter->second->GetHP() <= 0)
				{
					printf("sibal! %d\n", iter->second->GetHP());
					iter->second->SetActive(false);
				}
			}
		}
	}


	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F4))
		{
			m_PlayerUMap->at(m_PlayerID)->SetActive(false);
		}
		if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F5))
		{
			m_PlayerUMap->at(m_PlayerID)->SetActive(true);
		}

		if (!m_PlayerUMap->at(m_PlayerID)->GetActive())
		{
			m_PlayerUMap->at(m_PlayerID)->DiedControl(pHID, pQuadTree, deltaTime);
			XMFLOAT3 tempPos = m_PlayerUMap->at(m_PlayerID)->GetPosition();
			tempPos.y += 20;
			pCamera->SetPosition(tempPos);
			pCamera->SetRotation(m_PlayerUMap->at(m_PlayerID)->GetRotation());
		}
	}
	m_PlayerUMapMutex.unlock();

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
		m_EventUMap->at(id)->m_FirstRender = false;
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

bool ModelManager::ModelPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	XMFLOAT3 pos;
	float height;

	/***** 모델 벡터 <뮤텍스> : 잠금 *****/
	m_ModelVecMutex.lock();
	for (auto iter = m_ModelVec->begin(); iter != m_ModelVec->end(); iter++)
	{
		pos = (*iter)->GetPosition();
		if (pQuadTree->GetHeightAtPosition(pos.x, pos.z, height))
		{
			(*iter)->SetPosition(XMFLOAT3(pos.x, height, pos.z));
		}
	}
	m_ModelVecMutex.unlock();
	/***** 모델 벡터 <뮤텍스> : 해제 *****/

	return true;
}

bool ModelManager::RecvGameOverPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	pNetworkEngine->GetRecvGOPQueueMutex().lock();
	while (!pNetworkEngine->GetRecvGOPQueue()->empty())
	{
		GameOverPacket gop = pNetworkEngine->GetRecvGOPQueue()->front();
		pNetworkEngine->GetRecvGOPQueue()->pop();
		if (gop.winner)
		{
			m_GameWinFlag = true;
		}
		else
		{
			m_GameOverFlag = true;
		}
	}
	pNetworkEngine->GetRecvGOPQueueMutex().unlock();

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
				if (iterPlayer->second->GetActive() && iterPlayer->second->m_FirstRender)
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
				if (iterEvent->second->GetActive() && iterEvent->second->m_FirstRender)
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
							printf("Collision!!!!!!!!!!!!!!!!\n");

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
			if (m_Monster->GetActive() && m_Monster->m_FirstRender)
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

bool ModelManager::EmotionSystem(Direct3D* pDirect3D, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, int screenWidth, int screenHeight)
{
	XMFLOAT3 vertex[8];

	int max = 0;
	int emotion[4];

	m_PlayerUMapMutex.lock();
	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
	{
		if (m_PlayerUMap->at(m_PlayerID)->GetActive())
		{
			emotion[0] = m_PlayerUMap->at(m_PlayerID)->GetEmotion0();
			emotion[1] = m_PlayerUMap->at(m_PlayerID)->GetEmotion1();
			emotion[2] = m_PlayerUMap->at(m_PlayerID)->GetEmotion2();
			emotion[3] = m_PlayerUMap->at(m_PlayerID)->GetEmotion3();

			m_PlayerReduceEmotionCheck = clock();
			if ((m_PlayerReduceEmotionCheck - m_PlayerReduceEmotionInit) >= 1000)
			{
				m_PlayerReduceEmotionInit = clock();

				int reducedEmotion[4];
				for (int i = 0; i < 4; i++)
				{
					reducedEmotion[i] = emotion[i] - 1;
					if (reducedEmotion[i] < 0)
					{
						reducedEmotion[i] = 0;
					}
				}

				m_PlayerUMap->at(m_PlayerID)->SetEmotion0(reducedEmotion[0]);
				m_PlayerUMap->at(m_PlayerID)->SetEmotion1(reducedEmotion[1]);
				m_PlayerUMap->at(m_PlayerID)->SetEmotion2(reducedEmotion[2]);
				m_PlayerUMap->at(m_PlayerID)->SetEmotion3(reducedEmotion[3]);

				m_PlayerEmotion[0] = reducedEmotion[0];
				m_PlayerEmotion[1] = reducedEmotion[1];
				m_PlayerEmotion[2] = reducedEmotion[2];
				m_PlayerEmotion[3] = reducedEmotion[3];
			}
		}
	}
	m_PlayerUMapMutex.unlock();
	

	for (int i = 1; i <= 3; i++)
	{
		if (emotion[max] < emotion[i])
		{
			max = i;
		}
	}

	float radius = ((float)emotion[max] / 100.0f) * (screenWidth / 2.0f);
	XMFLOAT2 screenCenter = XMFLOAT2(static_cast<float>(screenWidth / 2), static_cast<float>(screenHeight / 2));
	XMFLOAT3 screenPos[8];

//	/***** Player-Player 감정 전달 : 시작 *****/
//	/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
//	m_PlayerUMapMutex.lock();
//	if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
//	{
//		for (auto iterPlayer = m_PlayerUMap->begin(); iterPlayer != m_PlayerUMap->end(); iterPlayer++)
//		{
//			if (iterPlayer->first == m_PlayerID)
//				continue;
//
//			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && iterPlayer->second->IsInitilized())
//			{
//				if (iterPlayer->second->GetActive())
//				{
//					for (unsigned int mc = 0; mc < iterPlayer->second->GetMeshCount(); mc++)
//					{
//						iterPlayer->second->GetBBVertex(mc, vertex);
//
//						for (int i = 0; i < 8; i++)
//						{
//							screenPos[i] = WorldToScreen(pDirect3D, pCamera, vertex[i], iterPlayer->second->GetWorldMatrixBB(mc), screenWidth, screenHeight);
//						}
//
//						if (screenPos[0].z > 1.0f || screenPos[1].z > 1.0f || screenPos[2].z > 1.0f || screenPos[3].z > 1.0f ||
//							screenPos[4].z > 1.0f || screenPos[5].z > 1.0f || screenPos[6].z > 1.0f || screenPos[7].z > 1.0f)
//						{
//							break;
//						}
//
//						XMFLOAT2 modelCenterPos;
//						float modelRadius;
//						FindMaxMin(screenPos, modelCenterPos, modelRadius);
//
//						if ((0 <= modelCenterPos.x && modelCenterPos.x <= screenWidth) && (0 <= modelCenterPos.y && modelCenterPos.y <= screenHeight) &&
//							(sqrt(pow(screenCenter.x - modelCenterPos.x, 2) + pow(screenCenter.y - modelCenterPos.y, 2)) <= (radius + modelRadius)))
//						{
//							/*printf("modelCenterPos = (%f, %f), raduis = %f, modelRadius = %f, sqrt = %f\n",
//								modelCenterPos.x,
//								modelCenterPos.y,
//								radius,
//								modelRadius,
//								sqrt(pow(screenCenter.x - modelCenterPos.x, 2) + pow(screenCenter.y - modelCenterPos.y, 2))
//							);
//
//							printf("z = (");
//							for (int i = 0; i < 8; i++)
//							{
//								printf("%f ", screenPos[i].z);
//							}
//							printf(")\n");*/
//
//							m_PlayerEmotionCheck = clock();
//							if ((m_PlayerEmotionCheck - m_PlayerEmotionInit) >= 1000)
//							{
//								m_PlayerEmotionInit = clock();
//
//								Player2Player p2p;
//
//								p2p.player1Id = m_PlayerID;
//								p2p.player2Id = iterPlayer->first;
//
//								if (emotion[max] > 0)
//								{
//									p2p.emotion[max] = 2;
//									emotion[max] -= 2;
//									if (emotion[max] < 0)
//									{
//										emotion[max] = 0;
//									}
//
//									if (max == 0)
//									{
//										m_PlayerUMap->at(m_PlayerID)->SetEmotion0(emotion[max]);
//									}
//									else if (max == 1)
//									{
//										m_PlayerUMap->at(m_PlayerID)->SetEmotion1(emotion[max]);
//									}
//									else if (max == 2)
//									{
//										m_PlayerUMap->at(m_PlayerID)->SetEmotion2(emotion[max]);
//									}
//									else if (max == 3)
//									{
//										m_PlayerUMap->at(m_PlayerID)->SetEmotion3(emotion[max]);
//									}
//								}
//								else
//								{
//									break;
//								}
//
//								/***** 플레이어-플레이어 큐 <뮤텍스> : 잠금 *****/
//								pNetworkEngine->GetSendP2PQueueMutex().lock();
//								pNetworkEngine->GetSendP2PQueue()->push(p2p);
//								pNetworkEngine->GetSendP2PQueueMutex().unlock();
//								/***** 플레이어-플레이어 큐 <뮤텍스> : 해제 *****/
//#ifdef _DEBUG
//								printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendP2PQueue()->push(p2p)\n");
//#endif
//							}
//						}
//
//					}
//				}
//			}
//		}
//	}
//	m_PlayerUMapMutex.unlock();
//	/***** 플레이어 U맵 <뮤텍스> : 해제 *****/
//	/***** Player-Player 감정 전달 : 종료 *****/

	/***** Player-Monster 감정 상승 : 시작 *****/
	m_MonsterEmotionCheck = clock();
	if ((m_MonsterEmotionCheck - m_MonsterEmotionInit) >= 1000)
	{
		m_MonsterEmotionInit = clock();

		/***** 몬스터 <뮤텍스> : 잠금 *****/
		m_MonsterMutex.lock();
		/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
		m_PlayerUMapMutex.lock();
		if (m_PlayerUMap->find(m_PlayerID) != m_PlayerUMap->end())
		{
			if (m_PlayerUMap->at(m_PlayerID)->IsInitilized() && m_Monster->IsInitilized())
			{
				m_PlayerUMapMutex.unlock();
				/***** 플레이어 U맵 <뮤텍스> : 해제 *****/

				if (m_Monster->GetActive())
				{
					for (unsigned int mc = 0; mc < m_Monster->GetMeshCount(); mc++)
					{
						m_Monster->GetBBVertex(mc, vertex);

						for (int i = 0; i < 8; i++)
						{
							screenPos[i] = WorldToScreen(pDirect3D, pCamera, vertex[i], m_Monster->GetWorldMatrixBB(mc), screenWidth, screenHeight);
							//printf("i = %d, screenPos = (%f, %f, %f)\n", i, screenPos[i].x, screenPos[i].y, screenPos[i].z);
						}

						if (screenPos[0].z > 1.0f || screenPos[1].z > 1.0f || screenPos[2].z > 1.0f || screenPos[3].z > 1.0f ||
							screenPos[4].z > 1.0f || screenPos[5].z > 1.0f || screenPos[6].z > 1.0f || screenPos[7].z > 1.0f)
						{
							break;
						}

						XMFLOAT2 modelCenterPos;
						float modelRadius;
						FindMaxMin(screenPos, modelCenterPos, modelRadius);

						if ((0 <= modelCenterPos.x && modelCenterPos.x <= screenWidth) && (0 <= modelCenterPos.y && modelCenterPos.y <= screenHeight) &&
							(sqrt(pow(screenCenter.x - modelCenterPos.x, 2) + pow(screenCenter.y - modelCenterPos.y, 2)) <= (radius + modelRadius))) 
						{
							Player2Monster p2m;

							emotion[0] = m_PlayerUMap->at(m_PlayerID)->GetEmotion0();
							emotion[1] = m_PlayerUMap->at(m_PlayerID)->GetEmotion1();
							emotion[2] = m_PlayerUMap->at(m_PlayerID)->GetEmotion2();
							emotion[3] = m_PlayerUMap->at(m_PlayerID)->GetEmotion3();

							max = 0;

							for (int i = 1; i <= 3; i++)
							{
								if (emotion[max] < emotion[i])
								{
									max = i;
								}
							}

							if (emotion[max] > 0)
							{
								p2m.emotion[max] = 3;
							}
							else
							{
								break;
							}

							/***** 플레이어-몬스터 큐 <뮤텍스> : 잠금 *****/
							pNetworkEngine->GetSendP2MQueueMutex().lock();
							pNetworkEngine->GetSendP2MQueue()->push(p2m);
							pNetworkEngine->GetSendP2MQueueMutex().unlock();
							/***** 플레이어-몬스터 큐 <뮤텍스> : 해제 *****/
#ifdef _DEBUG
							printf("PUSH >> ModelManager.cpp : pNetworkEngine->GetSendP2MQueue()->push(p2m)\n");
#endif

							break;
							
						}
					}
				}
			
			/***** 플레이어 U맵 <뮤텍스> : 잠금 *****/
			m_PlayerUMapMutex.lock();
			}
		}
		m_PlayerUMapMutex.unlock();
		/***** 플레이어 U맵 <뮤텍스> : 해제 *****/
		m_MonsterMutex.unlock();
		/***** 몬스터 <뮤텍스> : 해제 *****/
	}
	/***** Player-Monster 감정 상승 : 종료 *****/


	return true;
}

XMFLOAT3 ModelManager::WorldToScreen(Direct3D* pDirect3D, Camera* pCamera, XMFLOAT3 worldPosition, XMMATRIX worldMatrix, int screenWidth, int screenHeight)
{
	XMMATRIX viewM;
	pCamera->GetViewMatrix(viewM);

	XMMATRIX projM;
	pDirect3D->GetProjectionMatrix(projM);

	XMVECTOR worldPositionVec = XMLoadFloat3(&worldPosition);
	XMVECTOR screenVec = XMVector3Project(worldPositionVec, 0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f, projM, viewM, worldMatrix);

	XMFLOAT3 screenPosition;
	XMStoreFloat3(&screenPosition, screenVec);

	return screenPosition;
}

void ModelManager::FindMaxMin(XMFLOAT3 screenPos[8], XMFLOAT2& modelCenterPos, float& modelRadius)
{
	XMFLOAT2 min, max;

	min.x = screenPos[0].x;
	min.y = screenPos[0].y;
	max.x = min.x;
	max.y = min.y;

	for (int i = 1; i < 8; i++)
	{
		if (screenPos[i].x < min.x)
		{
			min.x = screenPos[i].x;
		}
		if (screenPos[i].y < min.y)
		{
			min.y = screenPos[i].y;
		}
		if (screenPos[i].x > max.x)
		{
			max.x = screenPos[i].x;
		}
		if (screenPos[i].y > max.y)
		{
			max.y = screenPos[i].y;
		}
	}
	
	modelCenterPos = XMFLOAT2((min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f);
	//modelRadius = sqrtf(pow(modelCenterPos.x - max.x, 2) + pow(modelCenterPos.y - max.y, 2));

	XMFLOAT2 howMin;
	howMin.x = abs(max.x - min.x) * 0.7f;
	howMin.y = abs(max.y - min.y) * 0.7f;
	modelRadius = howMin.x < howMin.y ? howMin.x : howMin.y;
}

//bool ModelManager::TestIntersection(Direct3D* pDirect3D, Camera* pCamera, int pointX, int pointY, int screenWidth, int screenHeight, XMFLOAT3 modelPosition, float screenRadius)
//{
//	XMMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
//	XMFLOAT3 direction, origin, rayOrigin, rayDirection;
//
//	// 마우스 커서 좌표를 -1에서 +1 범위로 이동합니다
//	float screenPointX = ((2.0f * (float)pointX) / (float)screenWidth) - 1.0f;
//	float screenPointY = (((2.0f * (float)pointY) / (float)screenHeight) - 1.0f) * -1.0f;
//
//	// 뷰포트의 종횡비를 고려하여 투영 행렬을 사용하여 점을 조정합니다
//	pDirect3D->GetProjectionMatrix(projectionMatrix);
//
//	XMFLOAT3X3 projectionMatrix4;
//	XMStoreFloat3x3(&projectionMatrix4, projectionMatrix);
//
//	screenPointX = screenPointX / projectionMatrix4._11;
//	screenPointY = screenPointY / projectionMatrix4._22;
//
//	// 뷰 행렬의 역함수를 구합니다.
//	pCamera->GetViewMatrix(viewMatrix);
//	inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);
//
//	XMFLOAT3X3 inverseViewMatrix4;
//	XMStoreFloat3x3(&inverseViewMatrix4, inverseViewMatrix);
//
//	// 뷰 공간에서 피킹 레이의 방향을 계산합니다.
//	direction.x = (screenPointX * inverseViewMatrix4._11) + (screenPointY * inverseViewMatrix4._21) + inverseViewMatrix4._31;
//	direction.y = (screenPointX * inverseViewMatrix4._12) + (screenPointY * inverseViewMatrix4._22) + inverseViewMatrix4._32;
//	direction.z = (screenPointX * inverseViewMatrix4._13) + (screenPointY * inverseViewMatrix4._23) + inverseViewMatrix4._33;
//
//	// 카메라의 위치 인 picking ray의 원점을 가져옵니다.
//	origin = pCamera->GetPosition();
//
//	// 세계 행렬을 가져와 구의 위치로 변환합니다.
//	pDirect3D->GetWorldMatrix(worldMatrix);
//	translateMatrix = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
//	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);
//
//	// 이제 번역 된 행렬의 역함수를 구하십시오.
//	inverseWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);
//
//	// 이제 광선 원점과 광선 방향을 뷰 공간에서 월드 공간으로 변환합니다.
//	XMStoreFloat3(&rayOrigin, XMVector3TransformCoord(XMVectorSet(origin.x, origin.y, origin.z, 0.0f), inverseWorldMatrix));
//	XMStoreFloat3(&direction, XMVector3TransformNormal(XMVectorSet(direction.x, direction.y, direction.z, 0.0f), inverseWorldMatrix));
//
//	// 광선 방향을 표준화합니다.
//	XMStoreFloat3(&rayDirection, XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 0.0f)));
//
//	/***** 광선벡터와 물체-카메라벡터의 각도가 90도 이상이면 계산하지 않습니다. : 시작 *****/
//	XMFLOAT3 directionVector;
//	directionVector.x = modelPosition.x - origin.x;
//	directionVector.y = modelPosition.y - origin.y;
//	directionVector.z = modelPosition.z - origin.z;
//
//	XMFLOAT3 dot;
//	XMStoreFloat3(&dot, XMVector3Dot(XMLoadFloat3(&directionVector), XMLoadFloat3(&rayDirection)));
//	if (dot.x < 0.0f)
//	{
//		return false;
//	}
//	/***** 광선벡터와 물체-카메라벡터의 각도가 90도 이상이면 계산하지 않습니다. : 종료 *****/
//
//	// 이제 광선 구 교차 테스트를 수행하십시오.
//	if (RaySphereIntersect(rayOrigin, rayDirection, screenRadius) == true)
//	{
//		//printf("RaySphereIntersect\n");
//		return true;
//	}
//
//	return false;
//}
//
//bool ModelManager::RaySphereIntersect(XMFLOAT3 rayOrigin, XMFLOAT3 rayDirection, float radius)
//{
//	// a, b 및 c 계수를 계산합니다.
//	float a = (rayDirection.x * rayDirection.x) + (rayDirection.y * rayDirection.y) + (rayDirection.z * rayDirection.z);
//	float b = ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) + (rayDirection.z * rayOrigin.z)) * 2.0f;
//	float c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) + (rayOrigin.z * rayOrigin.z)) - (radius * radius);
//
//	// 결과값을 얻는다
//	float discriminant = (b * b) - (4 * a * c);
//
//	// 결과값이 음수이면 피킹 선이 구를 벗어난 것입니다. 그렇지 않으면 구를 선택합니다.
//	if (discriminant < 0.0f)
//	{
//		return false;
//	}
//
//	return true;
//}
///***** 피킹 : 종료 *****/