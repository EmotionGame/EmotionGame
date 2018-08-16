#pragma once

class Direct3D;
class NetworkEngine;
class Camera;
class HID;
class Model;
class Player;
class Event;
class Monster;
class Object;
class QuadTree;
class Emotion;

class ModelManager : public AlignedAllocationPolicy<16>
{
public:
	unsigned int static __stdcall InitPlayerModelThread(void* p);
	UINT WINAPI _InitPlayerModelThread();
	unsigned int static __stdcall CopyPlayerModelThread(void* p);
	UINT WINAPI _CopyPlayerModelThread();

	unsigned int static __stdcall InitEventModelThread(void* p);
	UINT WINAPI _InitEventModelThread();

	unsigned int static __stdcall InitMonsterModelThread(void* p);
	UINT WINAPI _InitMonsterModelThread();

	unsigned int static __stdcall InitObjectModelThread(void* p);
	UINT WINAPI _InitObjectModelThread();

	unsigned int static __stdcall InitModelThread(void* p);
	UINT WINAPI _InitModelThread();

	ModelManager();
	ModelManager(const ModelManager& other);
	~ModelManager();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, 
		XMMATRIX baseViewMatrix, XMMATRIX orthoMatrix, 
		XMFLOAT3 cameraPosition, float deltaTime, int screenWidth, int screenHeight);
	bool Physics(Direct3D* pDirect3D, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime, int screenWidth, int screenHeight);

	int GetPlayerID();
	int GetPlayerHP();
	float GetPlayerSpeed();
	int* GetPlayerEmotion();

private:
	void DetectChangingValue(int playerID);

	bool PlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool CreatePlayerPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvUserPacket31Physics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvActionPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool PlayerActionPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool CheckPlayerActive(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool EventPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvEventPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool SendEventPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool MonsterPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);
	bool RecvMonsterPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool ModelPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool RecvGameOverPacketPhysics(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool CollisionDetection(HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, QuadTree* pQuadTree, float deltaTime);

	bool EmotionSystem(Direct3D* pDirect3D, HID* pHID, NetworkEngine* pNetworkEngine, Camera* pCamera, int screenWidth, int screenHeight);
	XMFLOAT3 WorldToScreen(Direct3D* pDirect3D, Camera* pCamera, XMFLOAT3 worldPosition, XMMATRIX worldMatrix, int screenWidth, int screenHeight);
	void FindMaxMin(XMFLOAT3 screenPos[8], XMFLOAT2& modelCenterPos, float& modelRadius);

private:
	ID3D11Device* m_device = nullptr; // 포인터를 받아와서 사용하므로 m_device->Shutdown() 금지
	HWND m_hwnd;

	Emotion* m_Emotion = nullptr;

	Emotion* m_GameWin = nullptr;
	Emotion* m_GameOver = nullptr;
	bool m_GameWinFlag = false;
	bool m_GameOverFlag = false;

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

	int m_PlayerID = -1;
	bool m_SetPlayerID = false;

	XMFLOAT3 m_PlayerPastPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPastRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_PlayerPresentRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

	bool m_DetectChanging[PLAYER_SIZE];
	clock_t m_PlayerActionInit = 0;
	clock_t m_PlayerActionCheck = 0;
	clock_t m_PlayerEmotionInit = 0;
	clock_t m_PlayerEmotionCheck = 0;
	clock_t m_PlayerReduceEmotionInit = 0;
	clock_t m_PlayerReduceEmotionCheck = 0;

	int m_PlayerHP = 100;
	float m_PlayerSpeed = 10.0f;
	int m_PlayerEmotion[4] = { 0, 0, 0, 0 };

#define EVENT_SIZE 7
	unsigned int m_InitEventCount = 0;
	std::mutex m_InitEventCountMutex; // m_InitEventCount 뮤텍스
	std::unordered_map<unsigned int, Event*>* m_EventUMap = nullptr;
	std::mutex m_EventUMapMutex; // m_EventUMap 뮤텍스
	clock_t m_FixedEventInit = 0;
	clock_t m_FixedEventCheck = 0;

#define MONSTER_SIZE 1
	Monster* m_Monster = nullptr;
	std::mutex m_MonsterMutex;
	clock_t m_MonsterInit = 0;
	clock_t m_MonsterCheck = 0;
	clock_t m_MonsterEmotionInit = 0;
	clock_t m_MonsterEmotionCheck = 0;

#define OBJECT_SIZE 6
	unsigned int m_InitObjectCount = 0;
	std::mutex m_InitObjectCountMutex; // m_InitObjectCount 뮤텍스
	std::unordered_map<unsigned int, Object*>* m_ObjectUMap = nullptr;
	std::mutex m_ObjectUMapMutex; // m_PlayerUMap 뮤텍스
	clock_t m_ObjectInit = 0;
	clock_t m_ObjectCheck = 0;
	clock_t m_ObjectEmotionInit = 0;
	clock_t m_ObjectEmotionCheck = 0;

#define MODEL_SIZE 6
	unsigned int m_InitModelCount = 0;
	std::mutex m_InitModelCountMutex; // m_InitEventCount 뮤텍스
	std::vector<Model*>* m_ModelVec = nullptr;
	std::mutex m_ModelVecMutex; // m_EventUMap 뮤텍스

	bool m_LineRenderFlag = false;
};