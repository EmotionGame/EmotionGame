/*****  ������: �輺��	(KimSungMin)							*****/
/*****  ����: ���ѹα�(Korea)									*****/
/*****	�з�: ȫ�ʹ��б� ����ķ�۽� ���Ӽ���Ʈ���� 3�г� ������		*****/
/*****	���Ƹ�: Exdio(���� ���� �Ҹ���)							*****/
/*****	Phone: 010-8865-0312								*****/
/*****	GitHub: Minicastle									*****/
/*****  Blog: blog.naver.com/bloodxsecter					*****/
/*****  ���� ����: ���Ȧ Ŭ���̾�Ʈ ���α׷��� [180625-180817]	*****/
/*****  ���̼���(License): ����(Free)							*****/
/*****  Thank You!											*****/

#pragma once

class SystemInfo;
class HID;
class RenderingEngine;
class PhysicsEngine;
class NetworkEngine;

enum FRAME_STATE
{
	FPS_1,
	FPS_5,
	FPS_10,
	FPS_15,
	FPS_24,
	FPS_30,
	FPS_60,
	FPS_120,
	FPS_144,
	FPS_240,
	UN_LIMIT
};

class GameEngine : public AlignedAllocationPolicy<16>
{
public:
	GameEngine();
	GameEngine(const GameEngine& other);
	~GameEngine();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	HWND GetHWND();

	SystemInfo* GetSystemInfo();
	HID* GetHID();
	RenderingEngine* GetRenderingEngine();
	PhysicsEngine* GetPhysicsEngine();
	NetworkEngine* GetNetworkEngine();

	int& GetCpuPercentage();
	float& GetDeltaTime();

private:
	bool Frame();
	void InitializeWindows(int& screenWidth, int& screenHeight);
	void ShutdownWindows();
	void FrameConstraintProcess();

private:
	HWND m_hwnd;
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;

	unsigned int m_screenWidth = 800; // 1600
	unsigned int m_screenHeight = 600; // 900

#define FRAME_THREAD_SIZE 4
	SystemInfo* m_SystemInfo = nullptr;
	HID* m_HID = nullptr;
	RenderingEngine* m_RenderingEngine = nullptr;
	PhysicsEngine* m_PhysicsEngine = nullptr;
	NetworkEngine* m_NetworkEngine = nullptr;

	int m_CpuPercentage = 0;
	float m_DeltaTime = 0.016f;
	
	/***** �ִ� ������ ���� : ���� *****/
#define FRAME_CONSTRAINT_SIZE 4
	float m_FrameTimeSum[FRAME_CONSTRAINT_SIZE] = { 0.0f, 0.0f, 0.0f, 0.0f };
	int m_FrameState[FRAME_CONSTRAINT_SIZE] = {
		FRAME_STATE::FPS_240, // HID
		FRAME_STATE::FPS_144, // RenderingEngine
		FRAME_STATE::FPS_120, // PhysicsEngine
		FRAME_STATE::FPS_1	  // NetworkEngine
	};
	float m_FrameTimeLimit[4];
	/***** �ּ� ������ ���� : ���� *****/
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static GameEngine* ApplicationHandle = 0;