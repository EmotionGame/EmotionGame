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
	
	/***** �÷��̾� ť ���ؽ� : ��� *****/
	m_PlayerQueueMutex.lock();
	m_InitPlayerQueue->push(player);
	m_PlayerQueueMutex.unlock();
	/***** �÷��̾� ť ���ؽ� : ���� *****/

	/***** �������� : ���� *****/
	WaitForSingleObject(m_InitSemaphore, INFINITE);
	player->Initialize(m_device, m_hwnd,
		"Data/KSM/X_Bot/X_Bot", L"Data/KSM/Default/Default_1.dds",
		XMFLOAT3(0.001f, 0.001f, 0.001f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), false, 0);
	ReleaseSemaphore(m_InitSemaphore, 1, NULL);
	/***** �������� : ���� *****/

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

	// ���ÿ� �ִ� 4�� �����常 ���� �ε��ϵ��� �����ϴ� ��������
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
	/***** �÷��̾� U�� ���ؽ� : ��� *****/
	m_PlayerUMapMutex.lock();
	for (auto iter = m_PlayerUMap->begin(); iter != m_PlayerUMap->end(); iter++)
	{
		if (iter->second->IsInitilized())
		{
			iter->second->Render(pDeviceContext, viewMatrix, projectionMatrix, cameraPosition, deltaTime);
		}
	}
	m_PlayerUMapMutex.unlock();
	/***** �÷��̾� U�� ���ؽ� : ���� *****/

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
	// ��Ʈ��ũ ���ῡ �������� ���� ����
	if (pNetworkEngine->GetConnectFlag())
	{
		/***** ������Ŷ �ʱ�ȭ ť ���ؽ� : ��� *****/
		pNetworkEngine->GetRecvUPQueueMutex().lock();
		while (!pNetworkEngine->GetRecvUPQueue()->empty()) // GetRecvUPQueue�� �� ������ ����
		{
			UserPacket player = pNetworkEngine->GetRecvUPQueue()->front();
			pNetworkEngine->GetRecvUPQueue()->pop();
			pNetworkEngine->GetRecvUPQueueMutex().unlock();
			/***** ������Ŷ �ʱ�ȭ ť ���ؽ� : ���� *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_RecvUserPacketQueue->pop();\n");
#endif
			int id = player.id;

			// Ű�� �Ѱ��� id�� �ش�Ǵ� ���Ұ� ���� �ʱ�ȭ �س��� ���� ���� ������

			/***** �÷��̾� ť ���ؽ�, �÷��̾� U�� ���ؽ� : ��� *****/
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
			/***** �÷��̾� ť ���ؽ�, �÷��̾� U�� ���ؽ� : ���� *****/

			/***** ������Ŷ �ʱ�ȭ ť ���ؽ� : ��� *****/
			pNetworkEngine->GetRecvUPQueueMutex().lock();
		}
		pNetworkEngine->GetRecvUPQueueMutex().unlock();
		/***** ������Ŷ �ʱ�ȭ ť ���ؽ� : ���� *****/


		/***** �׼���Ŷ ť ���ؽ� : ��� *****/
		pNetworkEngine->GetRecvAPQueueMutex().lock();
		while (!pNetworkEngine->GetRecvAPQueue()->empty()) // GetRecvAPQueue�� �� ������ ����
		{
			ActionPacket player = pNetworkEngine->GetRecvAPQueue()->front();
			pNetworkEngine->GetRecvAPQueue()->pop();
			pNetworkEngine->GetRecvAPQueueMutex().unlock();
			/***** �׼���Ŷ ť ���ؽ� : ���� *****/
#ifdef _DEBUG
			printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");
#endif
			int id = player.id;

			/***** �÷��̾� U�� ���ؽ� : ��� *****/
			m_PlayerUMapMutex.lock();
			if ((m_PlayerUMap->find(id) != m_PlayerUMap->end()) && m_PlayerUMap->at(id)->IsActive()) // Ű�� �����ϰ� Ȱ��ȭ �����̸�
			{
				m_PlayerUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_PlayerUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			}
			m_PlayerUMapMutex.unlock();
			/***** �÷��̾� U�� ���ؽ� : ���� *****/

			/***** �׼���Ŷ ť ���ؽ� : ��� *****/
			pNetworkEngine->GetRecvAPQueueMutex().lock();
		}
		pNetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** �׼���Ŷ ť ���ؽ� : ���� *****/

		/***** �÷��̾� U�� ���ؽ� : ��� *****/
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
				/***** �÷��̾� U�� ���ؽ� : ���� *****/

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

					/***** �׼���Ŷ ť ���ؽ� : ��� *****/
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
					/***** �׼���Ŷ ť ���ؽ� : ���� *****/
				}
				/***** �÷��̾� U�� ���ؽ� : ��� *****/
				m_PlayerUMapMutex.lock();
			}
		}
		m_PlayerUMapMutex.unlock();
		/***** �÷��̾� U�� ���ؽ� : ���� *****/
	}

	return true;
}