#include "stdafx.h"
#include "Direct3D.h"
#include "FPS.h"
#include "HID.h"
#include "Camera.h"
#include "ModelManager.h"
#include "TextManager.h"
#include "NetworkEngine.h"
#include "SkyDome.h"
#include "SkyDomeShader.h"
#include "Terrain.h"
#include "TerrainShader.h"
#include "Frustum.h"
#include "QuadTree.h"
#include "RenderingEngine.h"

RenderingEngine::RenderingEngine()
{
}

RenderingEngine::RenderingEngine(const RenderingEngine& other)
{
}

RenderingEngine::~RenderingEngine()
{
}

bool RenderingEngine::Initialize(HWND hwnd, HID* pHID, NetworkEngine* pNetworkEngine, int screenWidth, int screenHeight)
{
#ifdef _DEBUG
	printf("Start >> RenderingEngine.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;
	m_HID = pHID;
	m_NetworkEngine = pNetworkEngine;

	// Direct3D 객체 생성
	m_Direct3D = new Direct3D;
	if (!m_Direct3D)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Direct3D = new Direct3D;", L"Error", MB_OK);
		return false;
	}

	// Camera 객체 생성
	m_Camera = new Camera;
	if (!m_Camera)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Camera = new Camera;", L"Error", MB_OK);
		return false;
	}

	// ModelManager 객체 생성
	m_ModelManager = new ModelManager;
	if (!m_ModelManager)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_ModelManager = new ModelManager;", L"Error", MB_OK);
		return false;
	}

	// TextManager 객체 생성
	m_TextManager = new TextManager;
	if (!m_TextManager)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TextManager = new TextManager;", L"Error", MB_OK);
		return false;
	}

	// FPS 객체 생성
	m_FPS = new FPS;
	if (!m_FPS)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_FPS = new FPS;", L"Error", MB_OK);
		return false;
	}

	/***** SkyDome : 시작 *****/
	m_SkyDome = new SkyDome;
	if (!m_SkyDome)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_SkyDome = new SkyDome;", L"Error", MB_OK);
		return false;
	}

	m_SkyDomeShader = new SkyDomeShader;
	if (!m_SkyDomeShader)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_SkyDomeShader = new SkyDomeShader;", L"Error", MB_OK);
		return false;
	}
	/***** SkyDome : 종료 *****/

	/***** Terrrain : 시작 *****/
	m_Terrain = new Terrain;
	if (!m_Terrain)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Terrain = new Terrain;", L"Error", MB_OK);
		return false;
	}

	m_TerrainShader = new TerrainShader;
	if (!m_TerrainShader)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TerrainShader = new ColorShader;", L"Error", MB_OK);
		return false;
	}

	// frustum 객체를 생성합니다.
	m_Frustum = new Frustum;
	if (!m_Frustum)
	{
		return false;
	}

	// 쿼드 트리 객체를 생성합니다.
	m_QuadTree = new QuadTree;
	if (!m_QuadTree)
	{
		return false;
	}
	/***** Terrrain : 종료 *****/

	// Direct3D 객체 초기화
	if (!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, m_hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))", L"Error", MB_OK);
		return false;
	}

	// Camera 객체 초기화
	if (!m_Camera->Initialize(m_HID))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Camera->Initialize(m_HID)", L"Error", MB_OK);
		return false;
	}

	// 카메라 포지션 설정
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);

	// 초기화
	m_Camera->Render();

	// 초기 viewMatrix 저장
	m_Camera->GetViewMatrix(m_baseViewMatrix);

	// ModelManager 객체 초기화
	if (!m_ModelManager->Initialize(m_Direct3D->GetDevice(), m_hwnd, m_HID, pNetworkEngine, m_Camera, m_QuadTree))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_ModelManager->Initialize(m_Direct3D->GetDevice(), m_hwnd)", L"Error", MB_OK);
		return false;
	}

	// TextManager 객체 초기화
	if (!m_TextManager->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, m_baseViewMatrix))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TextManager->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, m_baseViewMatrix)", L"Error", MB_OK);
		return false;
	}

	// FPS 객체 초기화
	if (!m_FPS->Initialize())
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_FPS->Initialize()", L"Error", MB_OK);
		return false;
	}

	// 스카이 돔 객체를 초기화 합니다.
	if (!m_SkyDome->Initialize(m_Direct3D->GetDevice(), m_hwnd, "Data/SkyDome/SkyDome.txt", L"Data/SkyDome/Sunset.png"))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_SkyDome->Initialize()", L"Error", MB_OK);
		return false;
	}

	// 스카이 돔 쉐이더 객체를 초기화 합니다.
	if (!m_SkyDomeShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_SkyDomeShader->Initialize()", L"Error", MB_OK);
		return false;
	}

	if (!m_Terrain->Initialize(m_Direct3D->GetDevice(), "Data/Terrain/heightmap01.bmp", L"Data/Terrain/dirt01.dds"))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Terrain->Initialize", L"Error", MB_OK);
		return false;
	}

	if (!m_TerrainShader->Initialize(m_Direct3D->GetDevice(), m_hwnd))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TerrainShader->Initialize", L"Error", MB_OK);
		return false;
	}

	// 쿼드 트리 객체를 초기화 합니다.
	if (!m_QuadTree->Initialize(m_Terrain, m_Direct3D->GetDevice()))
	{
		MessageBox(hwnd, L"Could not initialize the quad tree object.", L"Error", MB_OK);
		return false;
	}

#ifdef _DEBUG
	printf("Success >> RenderingEngine.cpp : Initialize()\n");
#endif

	return true;
}

void RenderingEngine::Shutdown()
{
	// 쿼드 트리 객체를 해제합니다.
	if (m_QuadTree)
	{
		m_QuadTree->Shutdown();
		delete m_QuadTree;
		m_QuadTree = 0;
	}

	// frustum 객체를 해제합니다.
	if (m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	// 색상 셰이더 객체를 해제합니다.
	if (m_TerrainShader)
	{
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}

	// 지형 객체를 해제합니다.
	if (m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	// 스카이 돔 쉐이더 객체를 해제합니다.
	if (m_SkyDomeShader)
	{
		m_SkyDomeShader->Shutdown();
		delete m_SkyDomeShader;
		m_SkyDomeShader = 0;
	}

	// 스카이 돔 객체를 해제합니다.
	if (m_SkyDome)
	{
		m_SkyDome->Shutdown();
		delete m_SkyDome;
		m_SkyDome = 0;
	}

	// TextManager 객체 반환
	if (m_TextManager)
	{
		m_TextManager->Shutdown();
		delete m_TextManager;
		m_TextManager = nullptr;
	}

	// ModelManager 객체 반환
	if (m_ModelManager)
	{
		m_ModelManager->Shutdown();
		delete m_ModelManager;
		m_ModelManager = nullptr;
	}

	// Camera 객체 반환
	if (m_Camera)
	{
		m_Camera->Shutdown();
		delete m_Camera;
		m_Camera = nullptr;
	}

	// 포인터를 받아와서 사용하므로 m_HID->Shutdown() 금지

	// Direct3D 객체 반환
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}
}

bool RenderingEngine::Frame(int cputPercentage, float deltaTime)
{
	// 평균 델타 타임 구하는 함수
	CaluateAverageDeltaTime(deltaTime);

	if (!m_FPS->Frame())
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_FPS->Frame()", L"Error", MB_OK);
		return false;
	}

	if (!m_TextManager->Frame(m_Direct3D->GetDeviceContext(), cputPercentage, m_FPS->GetFps(), deltaTime, m_averageDeltaTime, m_ModelManager->GetPlayerID(), m_QuadTree->GetDrawCount()))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TextManager->Frame(m_Direct3D->GetDeviceContext(), cputPercentage, fps, deltaTime, averageDeltaTime)", L"Error", MB_OK);
		return false;
	}

	if (!m_Camera->Frame(deltaTime))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Camera->Frame()", L"Error", MB_OK);
		return false;
	}

	// 그래픽 랜더링 처리
	if (!Render(deltaTime))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : Render(deltaTime)", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool RenderingEngine::Render(float deltaTime)
{
	// 씬을 그리기 위해 버퍼를 지웁니다
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다
	m_Camera->Render();

	// 카메라 및 d3d 객체에서 월드, 뷰 및 투영 행렬을 가져옵니다
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);


	/***** 스카이돔 렌더링 : 시작 *****/
	// 표면 컬링을 끕니다.
	m_Direct3D->TurnOffCulling();

	// Z 버퍼를 끕니다.
	m_Direct3D->TurnZBufferOff();

	// 스카이 돔 셰이더를 사용하여 하늘 돔을 렌더링합니다.
	m_SkyDome->Render(m_Direct3D->GetDeviceContext(), deltaTime, m_Camera->GetPosition());
	m_SkyDomeShader->Render(m_Direct3D->GetDeviceContext(), m_SkyDome->GetIndexCount(), m_SkyDome->GetWorldMatrix(), viewMatrix, projectionMatrix, m_SkyDome->GetTexture());

	// Z 버퍼를 다시 켭니다.
	m_Direct3D->TurnZBufferOn();

	// 다시 표면 컬링을 되돌립니다.
	m_Direct3D->TurnOnCulling();
	/***** 스카이돔 렌더링 : 종료 *****/


	/***** 지형 렌더링 : 시작 *****/
	// 절두체를 생성합니다.
	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	// 렌더링에 사용할 지형 셰이더 매개 변수를 설정합니다.
	if (!m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(-0.5f, -1.0f, 0.0f), m_Terrain->GetTexture()))
	{
		return false;
	}

	// 쿼드 트리 및 지형 셰이더를 사용하여 지형을 렌더링합니다.
	m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);
	/***** 지형 렌더링 : 종료 *****/


	/***** 모델 렌더링 : 시작 *****/	
	m_ModelManager->Render(m_Direct3D->GetDeviceContext(), viewMatrix, projectionMatrix, m_Camera->GetPosition(), deltaTime); // 모델 매니저로부터 모델들을 렌더링합니다.
	/***** 모델 렌더링 : 종료 *****/


	/***** 텍스트 렌더링 : 시작 *****/
	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다.
	m_Direct3D->TurnZBufferOff();

	// 텍스트를 렌더링하기 전에 알파 블렌딩을 켭니다
	m_Direct3D->TurnOnAlphaBlending();

	// 텍스트 매니저로부터 텍스트들을 렌더링합니다.
	m_TextManager->Render(m_Direct3D->GetDeviceContext(), worldMatrix, orthoMatrix);

	// 텍스트를 렌더링 한 후 알파 블렌딩을 해제합니다
	m_Direct3D->TurnOffAlphaBlending();

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오.
	m_Direct3D->TurnZBufferOn();
	/***** 텍스트 렌더링 : 종료 *****/


	// 버퍼의 내용을 화면에 출력합니다
	m_Direct3D->EndScene();

	return true;
}

void RenderingEngine::CaluateAverageDeltaTime(float deltaTime)
{
	/***** 평균 델타 타임 구하는 구간 : 시작 *****/
	m_deltaTimeArr[m_deltaTimeCursor++] = deltaTime;

	if (!m_checkInitDeltaTime && (m_deltaTimeCursor == DELTA_TIME_SIZE))
		m_checkInitDeltaTime = true;

	if (m_deltaTimeCursor >= DELTA_TIME_SIZE) // #define DELTA_TIME_SIZE 1000
		m_deltaTimeCursor = 0;

	float deltaTimeSum = 0.0f;

	if (m_checkInitDeltaTime) {
		for (int i = 0; i < DELTA_TIME_SIZE; i++)
			deltaTimeSum += m_deltaTimeArr[i];
		m_averageDeltaTime = deltaTimeSum / (float)DELTA_TIME_SIZE;
	}
	else {
		for (int i = 0; i < m_deltaTimeCursor; i++)
			deltaTimeSum += m_deltaTimeArr[i];
		m_averageDeltaTime = deltaTimeSum / (float)m_deltaTimeCursor;
	}
	/***** 평균 델타 타임 구하는 구간 : 종료 *****/


}