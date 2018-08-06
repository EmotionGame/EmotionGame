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

	bool Initialize(ID3D11Device* pDevice, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime);
	bool Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	int GetPlayerID();

private:
	void DetectChangingValue(int playerID);
	bool PlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	unsigned int static __stdcall InitPlayerModelThread(void* p);
	UINT WINAPI _InitPlayerModelThread();

private:
	ID3D11Device* m_device = nullptr; // 포인터를 받아와서 사용하므로 m_device->Shutdown() 금지
	HWND m_hwnd;

	HANDLE m_InitSemaphore; // 동시에 최대 4개 스레드만 모델을 로드하도록 제한하는 세마포어

#define PLAYER_SIZE 8
	std::queue<Player*>* m_InitPlayerQueue = new std::queue<Player*>;
	std::mutex m_PlayerQueueMutex; // 
	std::unordered_map<int, Player*>* m_PlayerUMap = new std::unordered_map<int, Player*>;
	std::mutex m_PlayerUMapMutex; // 

	int m_PlayerID = 0;
	bool m_SetPlayerID = false;

	XMFLOAT3 m_PlayerPastPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPastRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool m_DetectChanging[PLAYER_SIZE];


};