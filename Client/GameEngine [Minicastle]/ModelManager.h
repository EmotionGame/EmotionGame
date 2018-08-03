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

	unsigned int static __stdcall InitModelThread(void* p);
	UINT WINAPI _InitModelThread();

private:
	ID3D11Device* m_device = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_device->Shutdown() ����
	HWND m_hwnd;
	HID* m_HID = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_HID->Shutdown() ����

	NetworkEngine* m_NetworkEngine = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_NetworkEngine->Shutdown() ����
	Camera* m_Camera = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_Camera->Shutdown() ����
	QuadTree* m_QuadTree = nullptr;

#define PLAYER_SIZE 8
	std::queue<Player*>* m_InitModels = new std::queue<Player*>;
	std::unordered_map<int, Player*>* m_ModelsUMap = new std::unordered_map<int, Player*>;

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

	// test
#define TEST_SIZE 1
	std::mutex countMutex;
	Model* m_tests[TEST_SIZE];
};