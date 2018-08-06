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
	WaitForSingleObject(m_InitSemaphore, INFINITE);

	Player* player = new Player;
	player->Initialize(m_device, m_hwnd, m_HID,
		"Data/KSM/PoliceOfficer/PoliceOfficer", L"Data/KSM/PoliceOfficer/PoliceOfficer.dds",
		XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -10.0f), false, 0);

	ReleaseSemaphore(m_InitSemaphore, 1, NULL);

	m_testQueueMutex.lock();
	m_testVector.push_back(player);
	m_testQueueMutex.unlock();

	_endthreadex(0);
	return true;
}

bool ModelManager::Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree)
{
#ifdef _DEBUG
	printf("Start >> ModelManager.cpp : Initialize()\n");
#endif
	m_device = pDevice;
	m_hwnd = hwnd;
	m_HID = pHID;

	m_NetworkEngine = pNetworkEngine;
	m_Camera = pCamera;
	m_QuadTree = pQuadTree;

	// 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어
	m_InitSemaphore = CreateSemaphore(NULL, 4, 4, NULL);

	for (int i = 0; i < 100; i++)
	{
		_beginthreadex(NULL, 0, InitPlayerModelThread, (LPVOID)this, 0, NULL);
	}

#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}
/***** 멀티 쓰레드 적용해야함 : 종료 *****/

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
	m_testQueueMutex.lock();
	for (auto iter = m_testVector.begin(); iter != m_testVector.end(); iter++)
	{
		if ((*iter)->IsInitilized())
		{
			(*iter)->Render(pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);
		}
	}
	m_testQueueMutex.unlock();

	// 네트워크 연결에 성공했을 때만 실행
	if (m_NetworkEngine->GetConnectFlag())
	{
		/***** m_RecvUPQueueMutex : 시작 *****/
		m_NetworkEngine->GetRecvUPQueueMutex().lock();

		// GetRecvUPQueue가 빌 때까지 실행
		while (!m_NetworkEngine->GetRecvUPQueue()->empty())
		{
			UserPacket player = m_NetworkEngine->GetRecvUPQueue()->front();
			m_NetworkEngine->GetRecvUPQueue()->pop();

			m_NetworkEngine->GetRecvUPQueueMutex().unlock();
			/***** m_RecvUPQueueMutex : 종료 *****/

			printf("POP >> ModelManager.cpp : m_RecvUserPacketQueue->pop();\n");

			int id = player.id;

			// 키로 넘겨준 id에 해당되는 원소가 없고 초기화 해놓은 모델이 남아 있으면
			if ((m_PlayerUMap->find(id) == m_PlayerUMap->end()) && !m_InitPlayerQueue->empty())
			{
				m_PlayerUMap->emplace(std::pair<int, Player*>(id, m_InitPlayerQueue->front()));
				m_InitPlayerQueue->pop();
				printf("POP >> ModelManager.cpp : m_InitPlayerQueue->pop();\n");

				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));

				if (!m_SetPlayerID)
				{
					m_PlayerID = id;
					m_SetPlayerID = true;
				}
			}

			/***** m_RecvUPQueueMutex : 시작 *****/
			m_NetworkEngine->GetRecvUPQueueMutex().lock();
		}

		m_NetworkEngine->GetRecvUPQueueMutex().unlock();
		/***** m_RecvUPQueueMutex : 종료 *****/


		/***** m_RecvAPQueueMutex : 시작 *****/
		m_NetworkEngine->GetRecvAPQueueMutex().lock();

		// GetRecvAPQueue가 빌 때까지 실행
		while (!m_NetworkEngine->GetRecvAPQueue()->empty())
		{
			ActionPacket player = m_NetworkEngine->GetRecvAPQueue()->front();
			m_NetworkEngine->GetRecvAPQueue()->pop();

			m_NetworkEngine->GetRecvAPQueueMutex().unlock();
			/***** m_RecvAPQueueMutex : 종료 *****/

			printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");

			int id = player.id;

			// 키가 존재하고 활성화 상태이면
			if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive())
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			}

			/***** m_RecvAPQueueMutex : 시작 *****/
			m_NetworkEngine->GetRecvAPQueueMutex().lock();
		}

		m_NetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** m_RecvAPQueueMutex : 종료 *****/


		for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
		{
			// 모델이 활성화 상태이면
			if (iter->second->IsActive())
			{
				// 플레이어의 ID와 같은 모델이라면
				if (iter->first == m_PlayerID)
				{
					iter->second->PlayerControl(deltaTime);
					m_PlayerPresentPos = iter->second->GetPosition();
					if (m_QuadTree->GetHeightAtPosition(m_PlayerPresentPos.x, m_PlayerPresentPos.z, m_PlayerPresentPos.y))
					{
						// 카메라 아래에 삼각형이 있는 경우 카메라를 두 개 단위로 배치합니다.
						iter->second->SetPosition(m_PlayerPresentPos);
					}
					m_PlayerPresentRot = iter->second->GetRotation();
					m_Camera->SetPosition(iter->second->GetCameraPosition());
					m_Camera->SetRotation(m_PlayerPresentRot);

					// 변경 감지
					DetectChangingValue(iter->first);

					// 변경이 되었을 때만
					if (m_DetectChanging[iter->first])
					{
						ActionPacket player;
						player.id = m_PlayerID;
						player.position[0] = m_PlayerPresentPos.x;
						player.position[1] = m_PlayerPresentPos.y;
						player.position[2] = m_PlayerPresentPos.z;
						player.rotation[0] = m_PlayerPresentRot.x;
						player.rotation[1] = m_PlayerPresentRot.y;
						player.rotation[2] = m_PlayerPresentRot.z;

						/***** m_SendAPQueueMutex : 시작 *****/
						m_NetworkEngine->GetSendAPQueueMutex().lock();

						if (m_NetworkEngine->GetSendAPQueue()->size() < QUEUE_LIMIT_SIZE)
						{
							m_NetworkEngine->GetSendAPQueue()->push(player);
							m_DetectChanging[iter->first] = false;
							printf("PUSH >> ModelManager.cpp : m_SendPacketQueue->push(player);\n");
						}

						m_NetworkEngine->GetSendAPQueueMutex().unlock();
						/***** m_SendAPQueueMutex : 종료 *****/
					}
				}

				// 렌더링
				if (!iter->second->Render(pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime))
				{
					MessageBox(m_hwnd, L"ModelManager.cpp : (*first)->Render(deviceContext, viewMatrix, projectionMatrix, cameraPosition)", L"Error", MB_OK);
					return false;
				}
			}
		}
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