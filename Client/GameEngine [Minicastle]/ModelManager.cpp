#include "stdafx.h"
#include "NetworkEngine.h"
#include "Camera.h"
#include "Model.h"
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

	/***** �÷��̾� �� : ���� *****/
	for (int i = 0; i < PLAYER_SIZE; i++)
	{
		m_InitModels->push(new Model);
		if (!m_InitModels->back())
		{
			MessageBox(m_hwnd, L"ModelManager.cpp : m_Models.back()", L"Error", MB_OK);
			return false;
		}
		if (!m_InitModels->back()->Initialize(m_device, m_hwnd, m_HID,
			"Data/FBX/PoliceOfficer/PoliceOfficer.fbx", L"Data/FBX/PoliceOfficer/PoliceOfficer.dds",
			XMFLOAT3(0.05f, 0.05f, 0.05f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), false))
		{
			MessageBox(m_hwnd, L"ModelManager.cpp : m_InitModels->back()->Initialize", L"Error", MB_OK);
			return false;
		}

		// ��ȭ ���� �ʱ�ȭ
		m_DetectChanging[i] = false;
	}
	/***** �÷��̾� �� : ���� *****/

	/***** ������Ʈ �� : ���� *****/

	/***** ������Ʈ �� : ���� *****/

	/***** �̺�Ʈ �� : ���� *****/
	/*m_InitEvents->push(new Model);
	if (!m_InitEvents->back())
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_Models.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InitEvents->back()->Initialize(m_device, m_hwnd, m_HID,
		"Data/FBX/Bonfire/Bonfire.fbx", L"Data/FBX/Default/Default_1.dds",
		XMFLOAT3(1.00f, 1.00f, 1.00f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(5.0f, 0.0f, 0.0f), false))
	{
		MessageBox(m_hwnd, L"ModelManager.cpp : m_InitEvents->back()->Initialize", L"Error", MB_OK);
		return false;
	}*/
	/***** �̺�Ʈ �� : ���� *****/

#ifdef _DEBUG
	printf("Success >> ModelManager.cpp : Initialize()\n");
#endif

	return true;
}
/***** ��Ƽ ������ �����ؾ��� : ���� *****/

void ModelManager::Shutdown()
{
	for (auto iter = m_ModelsUMap->begin(); iter != m_ModelsUMap->end(); iter++)
	{
		if (iter->second)
		{
			delete iter->second;
		}
	}
	m_ModelsUMap->clear();

	if (m_InitModels)
	{
		while (!m_InitModels->empty())
		{
			m_InitModels->front()->Shutdown();
			delete m_InitModels->front();
			m_InitModels->pop();
		}

		delete m_InitModels;
		m_InitModels = nullptr;
	}
}

bool ModelManager::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
{
	// ��Ʈ��ũ ���ῡ �������� ���� ����
	if (m_NetworkEngine->GetConnectFlag())
	{
		/***** m_RecvUPQueueMutex : ���� *****/
		m_NetworkEngine->GetRecvUPQueueMutex().lock();

		// GetRecvUPQueue�� �� ������ ����
		while (!m_NetworkEngine->GetRecvUPQueue()->empty())
		{
			UserPacket player = m_NetworkEngine->GetRecvUPQueue()->front();
			m_NetworkEngine->GetRecvUPQueue()->pop();

			m_NetworkEngine->GetRecvUPQueueMutex().unlock();
			/***** m_RecvUPQueueMutex : ���� *****/

			printf("POP >> ModelManager.cpp : m_RecvUserPacketQueue->pop();\n");

			int id = player.id;

			// Ű�� �Ѱ��� id�� �ش�Ǵ� ���Ұ� ���� �ʱ�ȭ �س��� ���� ���� ������
			if ((m_ModelsUMap->find(id) == m_ModelsUMap->end()) && !m_InitModels->empty())
			{
				m_ModelsUMap->emplace(std::pair<int, Model*>(id, m_InitModels->front()));
				m_InitModels->pop();
				printf("POP >> ModelManager.cpp : m_ModelsUMap[id] = m_InitModels->front();\n");

				m_ModelsUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_ModelsUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));

				if (!m_SetPlayerID)
				{
					m_PlayerID = id;
					m_SetPlayerID = true;
				}
			}

			/***** m_RecvUPQueueMutex : ���� *****/
			m_NetworkEngine->GetRecvUPQueueMutex().lock();
		}

		m_NetworkEngine->GetRecvUPQueueMutex().unlock();
		/***** m_RecvUPQueueMutex : ���� *****/


		/***** m_RecvAPQueueMutex : ���� *****/
		m_NetworkEngine->GetRecvAPQueueMutex().lock();

		// GetRecvAPQueue�� �� ������ ����
		while (!m_NetworkEngine->GetRecvAPQueue()->empty())
		{
			ActionPacket player = m_NetworkEngine->GetRecvAPQueue()->front();
			m_NetworkEngine->GetRecvAPQueue()->pop();

			m_NetworkEngine->GetRecvAPQueueMutex().unlock();
			/***** m_RecvAPQueueMutex : ���� *****/

			printf("POP >> ModelManager.cpp : m_RecvAactionPacketQueue->pop();\n");

			int id = player.id;

			// Ű�� �����ϰ� Ȱ��ȭ �����̸�
			if ((m_ModelsUMap->find(id) != m_ModelsUMap->end()) && m_ModelsUMap->at(id)->IsActive())
			{
				m_ModelsUMap->at(id)->SetPosition(XMFLOAT3(player.position[0], player.position[1], player.position[2]));
				m_ModelsUMap->at(id)->SetRotation(XMFLOAT3(player.rotation[0], player.rotation[1], player.rotation[2]));
			}

			/***** m_RecvAPQueueMutex : ���� *****/
			m_NetworkEngine->GetRecvAPQueueMutex().lock();
		}

		m_NetworkEngine->GetRecvAPQueueMutex().unlock();
		/***** m_RecvAPQueueMutex : ���� *****/


		for (auto iter = m_ModelsUMap->begin(); iter != m_ModelsUMap->end(); iter++)
		{
			// ���� Ȱ��ȭ �����̸�
			if (iter->second->IsActive())
			{
				// �÷��̾��� ID�� ���� ���̶��
				if (iter->first == m_PlayerID)
				{
					iter->second->PlayerControl(deltaTime);
					m_PlayerPresentPos = iter->second->GetPosition();
					if (m_QuadTree->GetHeightAtPosition(m_PlayerPresentPos.x, m_PlayerPresentPos.z, m_PlayerPresentPos.y))
					{
						// ī�޶� �Ʒ��� �ﰢ���� �ִ� ��� ī�޶� �� �� ������ ��ġ�մϴ�.
						iter->second->SetPosition(m_PlayerPresentPos);
					}
					m_PlayerPresentRot = iter->second->GetRotation();
					m_Camera->SetPosition(iter->second->GetCameraPosition());
					m_Camera->SetRotation(m_PlayerPresentRot);

					// ���� ����
					DetectChangingValue(iter->first);

					// ������ �Ǿ��� ����
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

						/***** m_SendAPQueueMutex : ���� *****/
						m_NetworkEngine->GetSendAPQueueMutex().lock();

						if (m_NetworkEngine->GetSendAPQueue()->size() < QUEUE_LIMIT_SIZE)
						{
							m_NetworkEngine->GetSendAPQueue()->push(player);
							m_DetectChanging[iter->first] = false;
							printf("PUSH >> ModelManager.cpp : m_SendPacketQueue->push(player);\n");
						}

						m_NetworkEngine->GetSendAPQueueMutex().unlock();
						/***** m_SendAPQueueMutex : ���� *****/
					}
				}

				// ������
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