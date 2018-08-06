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

#include "stdafx.h"
#include "SystemInfo.h"
#include "HID.h"
#include "RenderingEngine.h"
#include "PhysicsEngine.h"
#include "NetworkEngine.h"
#include "GameEngine.h"

//DWORD g_dwThreadID[FRAME_THREAD_SIZE];
//HANDLE g_hThread[FRAME_THREAD_SIZE];
//
//DWORD WINAPI ThreadProcSystemInfoFrame(LPVOID lpParam);
//DWORD WINAPI ThreadProcHIDFrame(LPVOID lpParam);
//DWORD WINAPI ThreadProcRenderingEngineFrame(LPVOID lpParam);
//DWORD WINAPI ThreadProcPhysicsEngineFrame(LPVOID lpParam);
//DWORD WINAPI ThreadProcNetworkEngineFrame(LPVOID lpParam);

GameEngine::GameEngine()
{
#ifdef _DEBUG
	// �ܼ�â ���� �ڵ�
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "wt", stdout);
#endif
}

GameEngine::GameEngine(const GameEngine& other)
{
}

GameEngine::~GameEngine()
{
#ifdef _DEBUG
	// �ܼ�â ����
	FreeConsole();
#endif
}

/***** ��Ƽ�����带 ���� �������־�� �� ��... *****/
bool GameEngine::Initialize()
{
#ifdef _DEBUG
	printf("Start >> GameEngine.cpp : Initialize()\n");
#endif

	// ������ â ����, ���� ���� ���� �ʱ�ȭ
	int screenWidth = 0;
	int screenHeight = 0;

	// ������ ���� �ʱ�ȭ
	InitializeWindows(screenWidth, screenHeight);

	// SystemInfo ��ü ����. CPU, FPS, Timer
	m_SystemInfo = new SystemInfo;
	if (!m_SystemInfo)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_SystemInfo = new SystemInfo;", L"Error", MB_OK);
		return false;
	}

	// HID ��ü ����. �� Ŭ������ ���� ������� Ű���� �Է� ó���� ���˴ϴ�.
	m_HID = new HID;
	if (!m_HID)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_HID = new HID;", L"Error", MB_OK);
		return false;
	}

	// RenderingEngine ��ü ����. �׷��� �������� ó���ϱ� ���� ��ü�Դϴ�.
	m_RenderingEngine = new RenderingEngine;
	if (!m_RenderingEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_RenderingEngine = new RenderingEngine;", L"Error", MB_OK);
		return false;
	}

	// PhysicsEngine ��ü ����. ��ü����, ������ ������ ó���ϱ� ���� ��ü�Դϴ�.
	m_PhysicsEngine = new PhysicsEngine;
	if (!m_PhysicsEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_PhysicsEngine = new PhysicsEngine;", L"Error", MB_OK);
		return false;
	}

	// NetworkEngine ��ü ����
	m_NetworkEngine = new NetworkEngine;
	if (!m_NetworkEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_NetworkEngine = new NetworkEngine;", L"Error", MB_OK);
		return false;
	}

	// SystemInfo ��ü �ʱ�ȭ.
	if (!m_SystemInfo->Initialize(m_hwnd))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_SystemInfo->Initialize(m_hwnd)", L"Error", MB_OK);
		return false;
	}

	// HID ��ü �ʱ�ȭ
	if (!m_HID->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_HID->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight)", L"Error", MB_OK);
		return false;
	}

	// RenderingEngine ��ü �ʱ�ȭ.
	if (!m_RenderingEngine->Initialize(m_hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_RenderingEngine->Initialize(m_hwnd, screenWidth, screenHeight)", L"Error", MB_OK);
		return false;
	}

	// PhysicsEngine ��ü �ʱ�ȭ.
	if (!m_PhysicsEngine->Initialize(m_hwnd))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_PhysicsEngine->Initialize(m_hwnd, m_HID)", L"Error", MB_OK);
		return false;
	}

	// NetworkEngine ��ü �ʱ�ȭ
	if (!m_NetworkEngine->Initialize(m_hwnd, "10.1.8.6", "8090")) // 10.1.8.6 : 10.1.7.107 : �� 172.30.1.20
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_NetworkEngine->Initialize", L"Error", MB_OK);
		return false;
	}
		
#ifdef _DEBUG
	printf("Success >> GameEngine.cpp : Initialize()\n");
#endif

	return true;
}

void GameEngine::Shutdown()
{
	// NetworkEngine ��ü ��ȯ
	if (m_NetworkEngine)
	{
		m_NetworkEngine->Shutdown();
		delete m_NetworkEngine;
		m_NetworkEngine = nullptr;
	}

	// PhysicsEngine ��ü ��ȯ
	if (m_PhysicsEngine)
	{
		m_PhysicsEngine->Shutdown();
		delete m_PhysicsEngine;
		m_PhysicsEngine = nullptr;
	}

	// RenderingEngine ��ü ��ȯ
	if (m_RenderingEngine)
	{
		m_RenderingEngine->Shutdown();
		delete m_RenderingEngine;	
		m_RenderingEngine = nullptr;
	}

	// HID ��ü ��ȯ
	if (m_HID)
	{
		m_HID->Shutdown();
		delete m_HID;
		m_HID = nullptr;
	}

	// SystemInfo ��ü ��ȯ
	if (m_SystemInfo)
	{
		m_SystemInfo->Shutdown();
		delete m_SystemInfo;
		m_SystemInfo = nullptr;
	}

	// Window ���� ó��
	ShutdownWindows();
}


void GameEngine::Run()
{
	// �޽��� ����ü ���� �� �ʱ�ȭ
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// ����ڷκ��� ���� �޽����� ���������� �޽��������� ���ϴ�
	while (true)
	{
		// ������ �޽����� ó���մϴ�
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// ���� �޽����� ���� ��� �޽��� ������ Ż���մϴ�
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// ����ڰ� ESCŰ�� �������� Ȯ�� �� ���� ó����
		if (m_HID->GetMouse_Keyboard()->IsEscapePressed() == true)
		{
			break;
		}
		else
		{
			// �� �ܿ��� Frame �Լ��� ó���մϴ�.
			if (!Frame())
				break;
		}
	}
}

LRESULT CALLBACK GameEngine::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

bool GameEngine::Frame()
{
	m_SystemInfo->Frame(m_CpuPercentage, m_DeltaTime);

	// �ִ� ������ ����
	FrameConstraintProcess();

	// HID
	if (m_FrameTimeSum[0] > m_FrameTimeLimit[0])
	{
		if (!m_HID->Frame())
		{
			MessageBox(m_hwnd, L"GameEngine.cpp : m_HID->Frame()", L"Error", MB_OK);
			return false;
		}
		m_FrameTimeSum[0] = 0.0f;
	}

	// RenderingEngine
	if (m_FrameTimeSum[1] > m_FrameTimeLimit[1])
	{
		if (!m_RenderingEngine->Frame(m_HID, m_CpuPercentage, m_FrameTimeSum[1]))
		{
			MessageBox(m_hwnd, L"GameEngine.cpp : m_RenderingEngine->Frame(m_CpuPercentage, m_FrameTimeSum[1])", L"Error", MB_OK);
			return false;
		}
		m_FrameTimeSum[1] = 0.0f;
	}

	// PhysicsEngine
	if (m_FrameTimeSum[2] > m_FrameTimeLimit[2])
	{
		if (!m_PhysicsEngine->Frame(m_RenderingEngine, m_HID, m_NetworkEngine, m_FrameTimeSum[2]))
		{
			MessageBox(m_hwnd, L"GameEngine.cpp : m_PhysicsEngine->Frame(m_FrameTimeSum[2])", L"Error", MB_OK);
			return false;
		}
		m_FrameTimeSum[2] = 0.0f;
	}

	// NetworkEngine
	if (m_FrameTimeSum[3] > m_FrameTimeLimit[3])
	{
		if (!m_NetworkEngine->Frame())
		{
			MessageBox(m_hwnd, L"GameEngine.cpp : m_NetworkEngine->Frame()", L"Error", MB_OK);
			return false;
		}
		m_FrameTimeSum[3] = 0.0f;
	}

	return true;
}

HWND GameEngine::GetHWND()
{
	return m_hwnd;
}

SystemInfo* GameEngine::GetSystemInfo()
{
	return m_SystemInfo;
}

HID* GameEngine::GetHID()
{
	return m_HID;
}

RenderingEngine* GameEngine::GetRenderingEngine()
{
	return m_RenderingEngine;
}

PhysicsEngine* GameEngine::GetPhysicsEngine()
{
	return m_PhysicsEngine;
}

NetworkEngine* GameEngine::GetNetworkEngine()
{
	return m_NetworkEngine;
}

int& GameEngine::GetCpuPercentage()
{
	return m_CpuPercentage;
}

float& GameEngine::GetDeltaTime()
{
	return m_DeltaTime;
}

void GameEngine::InitializeWindows(int& rScreenWidth, int& rScreenHeight)
{
	// �ܺ� �����͸� �� ��ü�� �����մϴ�
	ApplicationHandle = this;

	// �� ���α׷��� �ν��Ͻ��� �����ɴϴ�
	m_hinstance = GetModuleHandle(NULL);

	// ���α׷� �̸��� �����մϴ�
	m_applicationName = L"GameEngine by �輺��B [010-8865-0312]";

	// windows Ŭ������ �Ʒ��� ���� �����մϴ�.
	/*
	CS_HREDRAW : ������ �ʺ� �̵��ϰų� ����� �����Ǹ� �ٽ� �׸��ϴ�.
	CS_VREDRAW : ������ ���̰� �̵��ϰų� ����� �����Ǹ� �ٽ� �׸��ϴ�.
	CS_DBLCLKS : ���� Ŭ�� �޽����� �޽��ϴ�.
	*/
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL; // �޴��� ����
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// windows class�� ����մϴ�
	RegisterClassEx(&wc);

	// ����� ȭ���� �ػ󵵸� �о�ɴϴ�
	rScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	rScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = 0;
	int posY = 0;

	// FULL_SCREEN ���� ���� ���� ȭ���� �����մϴ�.
	if (FULL_SCREEN)
	{
		// Ǯ��ũ�� ���� �����ߴٸ� ����� ȭ�� �ػ󵵸� ����ũ�� �ػ󵵷� �����ϰ� ������ 32bit�� �����մϴ�.
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)rScreenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)rScreenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Ǯ��ũ������ ���÷��� ������ �����մϴ�.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}
	else
	{
		// ������ ����� ��� ũ�⸦ �����մϴ�.
		rScreenWidth = m_screenWidth;
		rScreenHeight = m_screenHeight;

		// ������ â�� ����, ������ �� ��� ������ �մϴ�.
		posX = (GetSystemMetrics(SM_CXSCREEN) - rScreenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - rScreenHeight) / 2;
	}

	// �����츦 �����ϰ� �ڵ��� ���մϴ�.
	/* 
	// dwStyle ���� �κ�
	WS_EX_APPWINDOW : �����찡 ���� �� ������ �½�ũ �� ���� �ֵ��� �մϴ�.
	WS_EX_ACCEPTFILES : �巡�� �Ǵ� ������ ���� �� �ֽ��ϴ�.
	WS_EX_CONTEXTHELP : Ÿ��Ʋ�ٿ� ? ��ư�� �����մϴ�.

	// dwStyle ���� �κ�
	WS_CLIPCHILDREN : �θ� �����츦 �׸��� �ڽ� ������� ��ġ�� �κ��� �׸��� �ʽ��ϴ�.
	WS_CLIPSIBLINGS : ���ϵ峢�� ��ȣ ��ģ ������ �׸��� �������� �����մϴ�.
	WS_POPUP : ������ Ÿ��Ʋ�� �����մϴ�.
	WS_SYSMENU : �ý��� �޴��� �����ϴ�. -> �ݱ� ��ư�� ����ϴ�.
	WS_THICKFRAME : ũ�⸦ ������ �� �ִ� ��輱�� �����ϴ�.
	*/
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU,
		posX, posY, rScreenWidth, rScreenHeight, NULL, NULL, m_hinstance, NULL);

	// �����츦 ȭ�鿡 ǥ���ϰ� ��Ŀ���� �����մϴ�
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
}


void GameEngine::ShutdownWindows()
{
	// Ǯ��ũ�� ��忴�ٸ� ���÷��� ������ �ʱ�ȭ�մϴ�.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// â�� �����մϴ�
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// ���α׷� �ν��Ͻ��� �����մϴ�
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// �ܺ������� ������ �ʱ�ȭ�մϴ�
	ApplicationHandle = NULL;
}

void GameEngine::FrameConstraintProcess()
{
	for (int i = 0; i < FRAME_CONSTRAINT_SIZE; i++)
	{
		m_FrameTimeSum[i] += m_DeltaTime;

		switch (m_FrameState[i])
		{
			case FRAME_STATE::FPS_1:
			{
				m_FrameTimeLimit[i] = 1000.0f;
				break;
			}
			case FRAME_STATE::FPS_5:
			{
				m_FrameTimeLimit[i] = 200.0f;
				break;
			}
			case FRAME_STATE::FPS_10:
			{
				m_FrameTimeLimit[i] = 100.0f;
				break;
			}
			case FRAME_STATE::FPS_15:
			{
				m_FrameTimeLimit[i] = 66.666f;
				break;
			}
			case FRAME_STATE::FPS_24:
			{
				m_FrameTimeLimit[i] = 41.666f;
				break;
			}
			case FRAME_STATE::FPS_30:
			{
				m_FrameTimeLimit[i] = 33.333f;
				break;
			}
			case FRAME_STATE::FPS_60:
			{
				m_FrameTimeLimit[i] = 16.666f;
				break;
			}
			case FRAME_STATE::FPS_120:
			{
				m_FrameTimeLimit[i] = 8.333f;
				break;
			}
			case FRAME_STATE::FPS_144:
			{
				m_FrameTimeLimit[i] = 6.944f;
				break;
			}
			case FRAME_STATE::FPS_240:
			{
				m_FrameTimeLimit[i] = 4.166f;
				break;
			}
			case FRAME_STATE::UN_LIMIT:
			{
				m_FrameTimeLimit[i] = 0.0f;
				break;
			}
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// ������ ���Ḧ Ȯ���մϴ�
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// �����찡 �������� Ȯ���մϴ�
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// �� ���� ��� �޽������� �ý��� Ŭ������ �޽��� ó���� �ѱ�ϴ�.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

//DWORD WINAPI ThreadProcSystemInfoFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// �ý��� ���� ������ ó���� �����մϴ�.
//	if (!GE->GetSystemInfo()->Frame(GE->GetCpuPercentage(), GE->GetFPS(), GE->GetDeltaTime(), GE->GetAverageDeltaTime()))
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_HID->Frame()", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}
//
//DWORD WINAPI ThreadProcHIDFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// �Է� ������ ó���� �����մϴ�.
//	if (!GE->GetHID()->Frame())
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_HID->Frame()", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}
//
//DWORD WINAPI ThreadProcRenderingEngineFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// �׷��� ������ Frame�� ó���մϴ�
//	if (!GE->GetRenderingEngine()->Frame(GE->GetCpuPercentage(), GE->GetFPS(), GE->GetDeltaTime(), GE->GetAverageDeltaTime(), NULL, nullptr))
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_RenderingEngine->Frame()", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}
//
//DWORD WINAPI ThreadProcPhysicsEngineFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// ���� ������ Frame�� ó���մϴ�.
//	if (!GE->GetPhysicsEngine()->Frame(GE->GetAverageDeltaTime()))
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_PhysicsEngine->Frame(deltaTime)", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}
//
//DWORD WINAPI ThreadProcNetworkEngineFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// ��Ʈ��ũ ������ Frame�� ó���մϴ�.
//	if (!GE->GetNetworkEngine()->Frame())
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_NetworkEngine->Frame()", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}