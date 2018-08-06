#pragma once

class Camera;
class Model;
class Player;
class NetworkEngine;
class QuadTree;

class ModelManager : public AlignedAllocationPolicy<16>
{
public:
	ModelManager();
	ModelManager(const ModelManager& other);
	~ModelManager();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime);

	int GetPlayerID();

private:
	void DetectChangingValue(int playerID);

	unsigned int static __stdcall InitPlayerModelThread(void* p);
	UINT WINAPI _InitPlayerModelThread();

private:
	ID3D11Device* m_device = nullptr; // 포인터를 받아와서 사용하므로 m_device->Shutdown() 금지
	HWND m_hwnd;
	HID* m_HID = nullptr; // 포인터를 받아와서 사용하므로 m_HID->Shutdown() 금지

	NetworkEngine* m_NetworkEngine = nullptr; // 포인터를 받아와서 사용하므로 m_NetworkEngine->Shutdown() 금지
	Camera* m_Camera = nullptr; // 포인터를 받아와서 사용하므로 m_Camera->Shutdown() 금지
	QuadTree* m_QuadTree = nullptr;

	HANDLE m_InitSemaphore; // 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어
	std::mutex m_testQueueMutex; // 
	std::vector<Player*> m_testVector;

#define PLAYER_SIZE 8
	std::queue<Player*>* m_InitPlayerQueue = new std::queue<Player*>;
	std::unordered_map<int, Player*>* m_PlayerUMap = new std::unordered_map<int, Player*>;
	std::mutex m_PlayerQueueMutex; // 

	int m_PlayerID = 0;
	bool m_SetPlayerID = false;

	XMFLOAT3 m_PlayerPastPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPastRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool m_DetectChanging[PLAYER_SIZE];

#define OBEJCT_SIZE 4
	std::queue<Model*>* m_InitObejcts = new std::queue<Model*>;
	std::unordered_map<int, Model*>* m_ObejectsUMap = new std::unordered_map<int, Model*>;

#define EVENT_SIZE 8
	std::queue<Model*>* m_InitEvents = new std::queue<Model*>;
	std::unordered_map<int, Model*>* m_EventUMap = new std::unordered_map<int, Model*>;


};