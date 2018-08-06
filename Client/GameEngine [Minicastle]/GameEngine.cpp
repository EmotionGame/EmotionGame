/*****  제작자: 김성민	(KimSungMin)							*****/
/*****  국적: 대한민국(Korea)									*****/
/*****	학력: 홍익대학교 세종캠퍼스 게임소프트웨어 3학년 재학중		*****/
/*****	동아리: Exdio(게임 제작 소모임)							*****/
/*****	Phone: 010-8865-0312								*****/
/*****	GitHub: Minicastle									*****/
/*****  Blog: blog.naver.com/bloodxsecter					*****/
/*****  인턴 경험: 블루홀 클라이언트 프로그래머 [180625-180817]	*****/
/*****  라이센스(License): 없음(Free)							*****/
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
	// 콘솔창 띄우는 코드
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
	// 콘솔창 해제
	FreeConsole();
#endif
}

/***** 멀티쓰레드를 각각 적용해주어야 할 듯... *****/
bool GameEngine::Initialize()
{
#ifdef _DEBUG
	printf("Start >> GameEngine.cpp : Initialize()\n");
#endif

	// 윈도우 창 가로, 세로 넓이 변수 초기화
	int screenWidth = 0;
	int screenHeight = 0;

	// 윈도우 생성 초기화
	InitializeWindows(screenWidth, screenHeight);

	// SystemInfo 객체 생성. CPU, FPS, Timer
	m_SystemInfo = new SystemInfo;
	if (!m_SystemInfo)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_SystemInfo = new SystemInfo;", L"Error", MB_OK);
		return false;
	}

	// HID 객체 생성. 이 클래스는 추후 사용자의 키보드 입력 처리에 사용됩니다.
	m_HID = new HID;
	if (!m_HID)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_HID = new HID;", L"Error", MB_OK);
		return false;
	}

	// RenderingEngine 객체 생성. 그래픽 랜더링을 처리하기 위한 객체입니다.
	m_RenderingEngine = new RenderingEngine;
	if (!m_RenderingEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_RenderingEngine = new RenderingEngine;", L"Error", MB_OK);
		return false;
	}

	// PhysicsEngine 객체 생성. 강체역학, 데이터 변경을 처리하기 위한 객체입니다.
	m_PhysicsEngine = new PhysicsEngine;
	if (!m_PhysicsEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_PhysicsEngine = new PhysicsEngine;", L"Error", MB_OK);
		return false;
	}

	// NetworkEngine 객체 생성
	m_NetworkEngine = new NetworkEngine;
	if (!m_NetworkEngine)
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_NetworkEngine = new NetworkEngine;", L"Error", MB_OK);
		return false;
	}

	// SystemInfo 객체 초기화.
	if (!m_SystemInfo->Initialize(m_hwnd))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_SystemInfo->Initialize(m_hwnd)", L"Error", MB_OK);
		return false;
	}

	// HID 객체 초기화
	if (!m_HID->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_HID->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight)", L"Error", MB_OK);
		return false;
	}

	// RenderingEngine 객체 초기화.
	if (!m_RenderingEngine->Initialize(m_hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_RenderingEngine->Initialize(m_hwnd, screenWidth, screenHeight)", L"Error", MB_OK);
		return false;
	}

	// PhysicsEngine 객체 초기화.
	if (!m_PhysicsEngine->Initialize(m_hwnd))
	{
		MessageBox(m_hwnd, L"GameEngine.cpp : m_PhysicsEngine->Initialize(m_hwnd, m_HID)", L"Error", MB_OK);
		return false;
	}

	// NetworkEngine 객체 초기화
	if (!m_NetworkEngine->Initialize(m_hwnd, "10.1.8.6", "8090")) // 10.1.8.6 : 10.1.7.107 : 집 172.30.1.20
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
	// NetworkEngine 객체 반환
	if (m_NetworkEngine)
	{
		m_NetworkEngine->Shutdown();
		delete m_NetworkEngine;
		m_NetworkEngine = nullptr;
	}

	// PhysicsEngine 객체 반환
	if (m_PhysicsEngine)
	{
		m_PhysicsEngine->Shutdown();
		delete m_PhysicsEngine;
		m_PhysicsEngine = nullptr;
	}

	// RenderingEngine 객체 반환
	if (m_RenderingEngine)
	{
		m_RenderingEngine->Shutdown();
		delete m_RenderingEngine;	
		m_RenderingEngine = nullptr;
	}

	// HID 객체 반환
	if (m_HID)
	{
		m_HID->Shutdown();
		delete m_HID;
		m_HID = nullptr;
	}

	// SystemInfo 객체 반환
	if (m_SystemInfo)
	{
		m_SystemInfo->Shutdown();
		delete m_SystemInfo;
		m_SystemInfo = nullptr;
	}

	// Window 종료 처리
	ShutdownWindows();
}


void GameEngine::Run()
{
	// 메시지 구조체 생성 및 초기화
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// 사용자로부터 종료 메시지를 받을때까지 메시지루프를 돕니다
	while (true)
	{
		// 윈도우 메시지를 처리합니다
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// 종료 메시지를 받을 경우 메시지 루프를 탈출합니다
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// 사용자가 ESC키를 눌렀는지 확인 후 종료 처리함
		if (m_HID->GetMouse_Keyboard()->IsEscapePressed() == true)
		{
			break;
		}
		else
		{
			// 그 외에는 Frame 함수를 처리합니다.
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

	// 최대 프레임 설정
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
	// 외부 포인터를 이 객체로 지정합니다
	ApplicationHandle = this;

	// 이 프로그램의 인스턴스를 가져옵니다
	m_hinstance = GetModuleHandle(NULL);

	// 프로그램 이름을 지정합니다
	m_applicationName = L"GameEngine by 김성민B [010-8865-0312]";

	// windows 클래스를 아래와 같이 설정합니다.
	/*
	CS_HREDRAW : 윈도우 너비가 이동하거나 사이즈가 조절되면 다시 그립니다.
	CS_VREDRAW : 윈도우 높이가 이동하거나 사이즈가 조절되면 다시 그립니다.
	CS_DBLCLKS : 더블 클릭 메시지를 받습니다.
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
	wc.lpszMenuName = NULL; // 메뉴바 설정
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// windows class를 등록합니다
	RegisterClassEx(&wc);

	// 모니터 화면의 해상도를 읽어옵니다
	rScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	rScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = 0;
	int posY = 0;

	// FULL_SCREEN 변수 값에 따라 화면을 설정합니다.
	if (FULL_SCREEN)
	{
		// 풀스크린 모드로 지정했다면 모니터 화면 해상도를 데스크톱 해상도로 지정하고 색상을 32bit로 지정합니다.
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)rScreenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)rScreenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// 풀스크린으로 디스플레이 설정을 변경합니다.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}
	else
	{
		// 윈도우 모드의 경우 크기를 지정합니다.
		rScreenWidth = m_screenWidth;
		rScreenHeight = m_screenHeight;

		// 윈도우 창을 가로, 세로의 정 가운데 오도록 합니다.
		posX = (GetSystemMetrics(SM_CXSCREEN) - rScreenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - rScreenHeight) / 2;
	}

	// 윈도우를 생성하고 핸들을 구합니다.
	/* 
	// dwStyle 설정 부분
	WS_EX_APPWINDOW : 윈도우가 보일 때 강제로 태스크 바 위에 있도록 합니다.
	WS_EX_ACCEPTFILES : 드래그 되는 파일을 받을 수 있습니다.
	WS_EX_CONTEXTHELP : 타이틀바에 ? 버튼을 생성합니다.

	// dwStyle 설정 부분
	WS_CLIPCHILDREN : 부모 윈도우를 그릴때 자식 윈도우와 겹치는 부분은 그리지 않습니다.
	WS_CLIPSIBLINGS : 차일드끼리 상호 겹친 영역은 그리기 영역에서 제외합니다.
	WS_POPUP : 윈도우 타이틀바 제거합니다.
	WS_SYSMENU : 시스템 메뉴를 가집니다. -> 닫기 버튼이 생깁니다.
	WS_THICKFRAME : 크기를 조절할 수 있는 경계선을 가집니다.
	*/
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU,
		posX, posY, rScreenWidth, rScreenHeight, NULL, NULL, m_hinstance, NULL);

	// 윈도우를 화면에 표시하고 포커스를 지정합니다
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
}


void GameEngine::ShutdownWindows()
{
	// 풀스크린 모드였다면 디스플레이 설정을 초기화합니다.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// 창을 제거합니다
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// 프로그램 인스턴스를 제거합니다
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// 외부포인터 참조를 초기화합니다
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
		// 윈도우 종료를 확인합니다
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// 윈도우가 닫히는지 확인합니다
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// 그 외의 모든 메시지들은 시스템 클래스의 메시지 처리로 넘깁니다.
	default:
	{
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

//DWORD WINAPI ThreadProcSystemInfoFrame(LPVOID lpParam) {
//	GameEngine* GE = static_cast<GameEngine*>(lpParam);
//	// 시스템 정보 프레임 처리를 수행합니다.
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
//	// 입력 프레임 처리를 수행합니다.
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
//	// 그래픽 엔진의 Frame을 처리합니다
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
//	// 물리 엔진의 Frame을 처리합니다.
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
//	// 네트워크 엔진의 Frame을 처리합니다.
//	if (!GE->GetNetworkEngine()->Frame())
//	{
//		MessageBox(GE->GetHWND(), L"GameEngine.cpp : m_NetworkEngine->Frame()", L"Error", MB_OK);
//		return false;
//	}
//
//	return true;
//}