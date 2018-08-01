#pragma once

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 3000.0f;
const float SCREEN_NEAR = 0.1f;

class Direct3D;
class FPS;
class HID;
class Camera;
class ModelManager;
class TextManager;
class NetworkEngine;
class SkyDome;
class SkyDomeShader;
class Terrain;
class TerrainShader;
class Frustum;
class QuadTree;

class RenderingEngine : public AlignedAllocationPolicy<16>
{
public:
	RenderingEngine();
	RenderingEngine(const RenderingEngine& other);
	~RenderingEngine();

	bool Initialize(HWND hwnd, HID* pHID, NetworkEngine* pNetworkEngine, int screenWidth, int screenHeight);
	void Shutdown();
	bool Frame(int cputPercentage, float deltaTime);

private:
	bool Render(float deltaTime);
	void CaluateAverageDeltaTime(float deltaTime);

private:
	HWND m_hwnd;

	Direct3D* m_Direct3D = nullptr;
	HID* m_HID = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_HID->Shutdown() ����
	FPS* m_FPS = nullptr;
	Camera* m_Camera = nullptr;
	XMMATRIX m_baseViewMatrix = XMMatrixIdentity();
	ModelManager* m_ModelManager = nullptr;
	TextManager* m_TextManager = nullptr;
	NetworkEngine* m_NetworkEngine = nullptr; // �����͸� �޾ƿͼ� ����ϹǷ� m_NetworkEngine->Shutdown() ����

	SkyDome* m_SkyDome = nullptr;
	SkyDomeShader* m_SkyDomeShader = nullptr;

	Terrain* m_Terrain = nullptr;
	TerrainShader* m_TerrainShader = nullptr;
	Frustum* m_Frustum = nullptr;
	QuadTree* m_QuadTree = nullptr;

	/***** ��� ��Ÿ Ÿ�� ���ϴ� ���� : ���� *****/
#define DELTA_TIME_SIZE 1000
	float m_deltaTimeArr[DELTA_TIME_SIZE];
	int m_deltaTimeCursor = 0;
	bool m_checkInitDeltaTime = false;
	float m_averageDeltaTime = 0.0f;
	/***** ��� ��Ÿ Ÿ�� ���ϴ� ���� : ���� *****/
};