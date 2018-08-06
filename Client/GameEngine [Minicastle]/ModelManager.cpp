#include "stdafx.h"
#include "NetworkEngine.h"
#include "Camera.h"
#include "Model.h"
#include "Player.h"
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

unsigned int __stdcall ModelManager::InitPlayerModelThread(void* p)
{
	static_cast<ModelManager*>(p)->_InitPlayerModelThread();

	return true;
}
UINT WINAPI ModelManager::_InitPlayerModelThread()
{
	Player* player = new Player;
	
	/***** 플레이어 큐 뮤텍스 : 잠금 *****/
	m_PlayerQueueMutex.lock();
	m_InitPlayerQueue->push(player);
	m_PlayerQueueMutex.unlock();
	/***** 플레이어 큐 뮤텍스 : 해제 *****/

	/***** 세마포어 : 진입 *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	player->Initialize(m_device, m_hwnd,
		"Data/KSM/X_Bot/X_Bot", L"Data/KSM/Default/Default_1.dds",
		XMFLOAT3(0.001f, 0.001f, 0.001f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), false, 0);
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** 세마포어 : 퇴장 *****/

	_endthreadex(0);
	return true;
}

bool ModelManager::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
#ifdef _DEBUG
	printf("Start >> ModelManager.cpp : Initialize()\n");
#endif
	m_device = pDevice;
	m_hwnd = hwnd;

	// 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어
	m_InitSemaphore = CreateSemaphore(NULL, 4, 4, NULL);

	for (int i = 0; i < PLAYER_SIZE; i++)
	{
		_beginthreadex(NULL, 0, InitPlayerModelThread, (LPVOID)this, 0, NULL);
	}

#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}

void ModelManager::Shutdown()
{
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		if (iter->second)
		{
			delete iter->second;
		}
	}
	m_PlayerUMap->clear();

	if (m_InitPlayerQueue)
	{
		while (!m_InitPlayerQueue->empty())
		{
			m_InitPlayerQueue->front()->Shutdown();
			delete m_InitPlayerQueue->front();
			m_InitPlayerQueue->pop();
		}

		delete m_InitPlayerQueue;
		m_InitPlayerQueue = nullptr;
	}
}

bool ModelManager::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
{ 
	/***** 플레이어 U맵 뮤텍스 : 잠금 *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		if (iter->second->IsInitilized())
		{
			iter->second->Render(pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** 플레이어 U맵 뮤텍스 : 해제 *****/

	return true;
}

bool ModelManager::Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime)
{
	PlayerPhysics(pHID, pNetworkEngine, pCamera, pQuadTree, deltaTime);

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
	// 네트워크 연결에 성공했을 때만 실행
	if (pNetworkEngine->GetConnectFlag())
	{
		/***** 유저패킷 초기화 큐 뮤텍스 : 잠금 *****/
		pNetworkEngine->GetRecvUPQueueMutex().lock();
		while (!pNetworkEngine->GetRecvUPQueue()->empty()) // GetRecvUPQueue가 빌 때까지 실행
		{
			UserPacket player = pNetworkEngine->GetRecvUPQueue()->front();
			pNetworkEngine->GetRecvUPQueue()->pop();
			pNetworkEngine->GetRecvUPQueueMutex().unlock();
			/***** 유저패킷 초기화 큐 뮤텍스 : 해제 *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_RecvUserPacketQueue->pop();\n");
#endif
			int id = player.id;

			// 키로 넘겨준 id에 해당되는 원소가 없고 초기화 해놓은 모델이 남아 있으면

			/***** 플레이어 큐 뮤텍스, 플레이어 U맵 뮤텍스 : 잠금 *****/
			m_PlayerQueueMutex.lock();
			m_PlayerUMapMutex.lock();

			if ((m_PlayerUMap->find(id) == m_PlayerUMap->end()) && !m_InitPlayerQueue->empty())
			{
				m_PlayerUMap->emplace(std::pair<int, Player*>(id, m_InitPlayerQueue->front()));
				m_InitPlayerQueue->pop();
#ifdef _DEBUG
				printf("POP >> ModelManager.cpp : m_InitPlayerQueue->pop();\n");
#endif
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));

				if (!m_SetPlayerID)
				{
					m_PlayerID = id;
					m_SetPlayerID = true;
				}
			}

			m_PlayerUMapMutex.unlock();
			m_PlayerQueueMutex.unlock();
			/***** 플레이어 큐 뮤텍스, 플레이어 U맵 뮤텍스 : 해제 *****/

			/***** 유저패킷 초기화 큐 뮤텍스 : 잠금 *****/
			pNetworkEngine->GetRecvUPQueueMutex().lock();
		}
		pNetworkEngine->GetRecvUPQueueMutex().unlock();
		/***** 유저패킷 초기화 큐 뮤텍스 : 해제 *****/


		/***** 액션패킷 큐 뮤텍스 : 잠금 *****/
		pNetworkEngine->GetRecvAPQueueMutex().lock();
		while (!pNetworkEngine->GetRecvAPQueue()->empty()) // GetRecvAPQueue가 빌 때까지 실행
		{
			ActionPacket player = pNetworkEngine->GetRecvAPQueue()->front();
			pNetworkEngine->GetRecvAPQueue()->pop();
			pNetworkEngine->GetRecvAPQueueMutex().unlock();
			/***** 액션패킷 큐 뮤텍스 : 해제 *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");
#endif
			int id = player.id;

			/***** 플레이어 U맵 뮤텍스 : 잠금 *****/
			m_PlayerUMapMutex.lock();
			if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // 키가 존재하고 활성화 상태이면
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			}
			m_PlayerUMapMutex.unlock();
			/***** 플레이어 U맵 뮤텍스 : 해제 *****/

			/***** 액션패킷 큐 뮤텍스 : 잠금 *****/
			pNetworkEngine->GetRecvAPQueueMutex().lock();
		}
		pNetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** 액션패킷 큐 뮤텍스 : 해제 *****/

		/***** 플레이어 U맵 뮤텍스 : 잠금 *****/
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
				/***** 플레이어 U맵 뮤텍스 : 해제 *****/

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

					/***** 액션패킷 큐 뮤텍스 : 잠금 *****/
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
					/***** 액션패킷 큐 뮤텍스 : 해제 *****/
				}
				/***** 플레이어 U맵 뮤텍스 : 잠금 *****/
				m_PlayerUMapMutex.lock();
			}
		}
		m_PlayerUMapMutex.unlock();
		/***** 플레이어 U맵 뮤텍스 : 해제 *****/
	}

	return true;
}