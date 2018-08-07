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
	ID3D11Device* m_device = nullptr; // 포인터를 받아와서 사용하므로 m_device->Shutdown() 금지
	HWND m_hwnd;

	HANDLE m_InitSemaphore; // 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어
	std::unordered_map<std::string, Model*>* m_AllModelUMap; // 모든 모델들을 저장하고 초기화까지 진행하는 UMap
	std::mutex m_AllModelUMapMutex; // m_AllModelUMap 뮤텍스

#define PLAYER_SIZE 8
	std::queue<Player*>* m_InitPlayerQueue = nullptr;
	std::mutex m_InitPlayerQueueMutex; // m_InitPlayerQueue 뮤텍스
	std::unordered_map<unsigned int, Player*>* m_PlayerUMap = nullptr;
	std::mutex m_PlayerUMapMutex; // m_PlayerUMap 뮤텍스
	bool m_PlayerInitilized = false; // m_AllModelUMap의 "Player"가 초기화되었는지 확인하는 플래그
	std::mutex m_PlayerInitilizedMutex; // m_PlayerInitilized 뮤텍스

	int m_PlayerID = 0;
	bool m_SetPlayerID = false;

	XMFLOAT3 m_PlayerPastPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPastRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool m_DetectChanging[PLAYER_SIZE];

#define EVENT_SIZE 7
	unsigned int m_InitEventCount = 0;
	std::mutex m_InitEventCountMutex; // m_InitEventCount 뮤텍스
	std::unordered_map<unsigned int, Event*>* m_EventUMap = nullptr;
	std::mutex m_EventUMapMutex; // m_EventUMap 뮤텍스
	// m_AllModelUMap의 각 "Event"가 초기화되었는지 확인하는 플래그
	bool m_EventInitilized[EVENT_SIZE] = { false, false, false, false, false, false, false };
	std::mutex m_EventInitilizedMutex[EVENT_SIZE]; // m_PlayerInitilized 뮤텍스

#define OBJECT_SIZE 6



#define MONSTER_SIZE 1


};