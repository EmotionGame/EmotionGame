#pragma once

class Direct3D;
class NetworkEngine;
class Camera;
class HID;
class Model;
class Player;
class Event;
class QuadTree;

class ModelManager : public AlignedAllocationPolicy<16>
{
public:
	ModelManager();
	ModelManager(const ModelManager& other);
	~ModelManager();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd);
	void Shutdown();
	bool Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime);
	bool Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	int GetPlayerID();

private:
	void DetectChangingValue(int playerID);

	bool PlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool CreatePlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvUserPacket31Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvActionPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool PlayerActionPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool CreateEventPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	unsigned int static __stdcall InitPlayerModelThread(void* p);
	UINT WINAPI _InitPlayerModelThread();
	unsigned int static __stdcall CopyPlayerModelThread(void* p);
	UINT WINAPI _CopyPlayerModelThread();

	unsigned int static __stdcall InitEventModelThread(void* p);
	UINT WINAPI _InitEventModelThread();

private:
	ID3D11Device* m_device = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_device->Shutdown() ����
	HWND m_hwnd;

	HANDLE m_InitSemaphore; // ���ÿ� �ִ� 4�� �����常 ���� �ε��ϵ��� �����ϴ� ��������
	std::unordered_map<std::string, Model*>* m_AllModelUMap; // ��� �𵨵��� �����ϰ� �ʱ�ȭ���� �����ϴ� UMap
	std::mutex m_AllModelUMapMutex; // m_AllModelUMap ���ؽ�

#define PLAYER_SIZE 8
	std::queue<Player*>* m_InitPlayerQueue = nullptr;
	std::mutex m_InitPlayerQueueMutex; // m_InitPlayerQueue ���ؽ�
	std::unordered_map<unsigned int, Player*>* m_PlayerUMap = nullptr;
	std::mutex m_PlayerUMapMutex; // m_PlayerUMap ���ؽ�
	bool m_PlayerInitilized = false; // m_AllModelUMap�� "Player"�� �ʱ�ȭ�Ǿ����� Ȯ���ϴ� �÷���
	std::mutex m_PlayerInitilizedMutex; // m_PlayerInitilized ���ؽ�

	int m_PlayerID = 0;
	bool m_SetPlayerID = false;

	XMFLOAT3 m_PlayerPastPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPastRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool m_DetectChanging[PLAYER_SIZE];

#define EVENT_SIZE 7
	unsigned int m_InitEventCount = 0;
	std::mutex m_InitEventCountMutex; // m_InitEventCount ���ؽ�
	std::unordered_map<unsigned int, Event*>* m_EventUMap = nullptr;
	std::mutex m_EventUMapMutex; // m_EventUMap ���ؽ�
	// m_AllModelUMap�� �� "Event"�� �ʱ�ȭ�Ǿ����� Ȯ���ϴ� �÷���
	bool m_EventInitilized[EVENT_SIZE] = { false, false, false, false, false, false, false };
	std::mutex m_EventInitilizedMutex[EVENT_SIZE]; // m_PlayerInitilized ���ؽ�

#define OBJECT_SIZE 6



#define MONSTER_SIZE 1


};