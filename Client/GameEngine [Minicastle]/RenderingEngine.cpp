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

	// Direct3D ��ü ����
	m_Direct3D = new Direct3D;
	if (!m_Direct3D)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Direct3D = new Direct3D;", L"Error", MB_OK);
		return false;
	}

	// Camera ��ü ����
	m_Camera = new Camera;
	if (!m_Camera)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Camera = new Camera;", L"Error", MB_OK);
		return false;
	}

	// ModelManager ��ü ����
	m_ModelManager = new ModelManager;
	if (!m_ModelManager)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_ModelManager = new ModelManager;", L"Error", MB_OK);
		return false;
	}

	// TextManager ��ü ����
	m_TextManager = new TextManager;
	if (!m_TextManager)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TextManager = new TextManager;", L"Error", MB_OK);
		return false;
	}

	// FPS ��ü ����
	m_FPS = new FPS;
	if (!m_FPS)
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_FPS = new FPS;", L"Error", MB_OK);
		return false;
	}

	/***** SkyDome : ���� *****/
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
	/***** SkyDome : ���� *****/

	/***** Terrrain : ���� *****/
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

	// frustum ��ü�� �����մϴ�.
	m_Frustum = new Frustum;
	if (!m_Frustum)
	{
		return false;
	}

	// ���� Ʈ�� ��ü�� �����մϴ�.
	m_QuadTree = new QuadTree;
	if (!m_QuadTree)
	{
		return false;
	}
	/***** Terrrain : ���� *****/

	// Direct3D ��ü �ʱ�ȭ
	if (!m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, m_hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))", L"Error", MB_OK);
		return false;
	}

	// Camera ��ü �ʱ�ȭ
	if (!m_Camera->Initialize(m_HID))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_Camera->Initialize(m_HID)", L"Error", MB_OK);
		return false;
	}

	// ī�޶� ������ ����
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);

	// �ʱ�ȭ
	m_Camera->Render();

	// �ʱ� viewMatrix ����
	m_Camera->GetViewMatrix(m_baseViewMatrix);

	// ModelManager ��ü �ʱ�ȭ
	if (!m_ModelManager->Initialize(m_Direct3D->GetDevice(), m_hwnd, m_HID, pNetworkEngine, m_Camera, m_QuadTree))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_ModelManager->Initialize(m_Direct3D->GetDevice(), m_hwnd)", L"Error", MB_OK);
		return false;
	}

	// TextManager ��ü �ʱ�ȭ
	if (!m_TextManager->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, m_baseViewMatrix))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_TextManager->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_hwnd, screenWidth, screenHeight, m_baseViewMatrix)", L"Error", MB_OK);
		return false;
	}

	// FPS ��ü �ʱ�ȭ
	if (!m_FPS->Initialize())
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_FPS->Initialize()", L"Error", MB_OK);
		return false;
	}

	// ��ī�� �� ��ü�� �ʱ�ȭ �մϴ�.
	if (!m_SkyDome->Initialize(m_Direct3D->GetDevice(), m_hwnd, "Data/SkyDome/SkyDome.txt", L"Data/SkyDome/Sunset.png"))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : m_SkyDome->Initialize()", L"Error", MB_OK);
		return false;
	}

	// ��ī�� �� ���̴� ��ü�� �ʱ�ȭ �մϴ�.
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

	// ���� Ʈ�� ��ü�� �ʱ�ȭ �մϴ�.
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
	// ���� Ʈ�� ��ü�� �����մϴ�.
	if (m_QuadTree)
	{
		m_QuadTree->Shutdown();
		delete m_QuadTree;
		m_QuadTree = 0;
	}

	// frustum ��ü�� �����մϴ�.
	if (m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	// ���� ���̴� ��ü�� �����մϴ�.
	if (m_TerrainShader)
	{
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}

	// ���� ��ü�� �����մϴ�.
	if (m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	// ��ī�� �� ���̴� ��ü�� �����մϴ�.
	if (m_SkyDomeShader)
	{
		m_SkyDomeShader->Shutdown();
		delete m_SkyDomeShader;
		m_SkyDomeShader = 0;
	}

	// ��ī�� �� ��ü�� �����մϴ�.
	if (m_SkyDome)
	{
		m_SkyDome->Shutdown();
		delete m_SkyDome;
		m_SkyDome = 0;
	}

	// TextManager ��ü ��ȯ
	if (m_TextManager)
	{
		m_TextManager->Shutdown();
		delete m_TextManager;
		m_TextManager = nullptr;
	}

	// ModelManager ��ü ��ȯ
	if (m_ModelManager)
	{
		m_ModelManager->Shutdown();
		delete m_ModelManager;
		m_ModelManager = nullptr;
	}

	// Camera ��ü ��ȯ
	if (m_Camera)
	{
		m_Camera->Shutdown();
		delete m_Camera;
		m_Camera = nullptr;
	}

	// �����͸� �޾ƿͼ� ����ϹǷ� m_HID->Shutdown() ����

	// Direct3D ��ü ��ȯ
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}
}

bool RenderingEngine::Frame(int cputPercentage, float deltaTime)
{
	// ��� ��Ÿ Ÿ�� ���ϴ� �Լ�
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

	// �׷��� ������ ó��
	if (!Render(deltaTime))
	{
		MessageBox(m_hwnd, L"RenderingEngine.cpp : Render(deltaTime)", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool RenderingEngine::Render(float deltaTime)
{
	// ���� �׸��� ���� ���۸� ����ϴ�
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// ī�޶��� ��ġ�� ���� �� ����� �����մϴ�
	m_Camera->Render();

	// ī�޶� �� d3d ��ü���� ����, �� �� ���� ����� �����ɴϴ�
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);


	/***** ��ī�̵� ������ : ���� *****/
	// ǥ�� �ø��� ���ϴ�.
	m_Direct3D->TurnOffCulling();

	// Z ���۸� ���ϴ�.
	m_Direct3D->TurnZBufferOff();

	// ��ī�� �� ���̴��� ����Ͽ� �ϴ� ���� �������մϴ�.
	m_SkyDome->Render(m_Direct3D->GetDeviceContext(), deltaTime, m_Camera->GetPosition());
	m_SkyDomeShader->Render(m_Direct3D->GetDeviceContext(), m_SkyDome->GetIndexCount(), m_SkyDome->GetWorldMatrix(), viewMatrix, projectionMatrix, m_SkyDome->GetTexture());

	// Z ���۸� �ٽ� �մϴ�.
	m_Direct3D->TurnZBufferOn();

	// �ٽ� ǥ�� �ø��� �ǵ����ϴ�.
	m_Direct3D->TurnOnCulling();
	/***** ��ī�̵� ������ : ���� *****/


	/***** ���� ������ : ���� *****/
	// ����ü�� �����մϴ�.
	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	// �������� ����� ���� ���̴� �Ű� ������ �����մϴ�.
	if (!m_TerrainShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(-0.5f, -1.0f, 0.0f), m_Terrain->GetTexture()))
	{
		return false;
	}

	// ���� Ʈ�� �� ���� ���̴��� ����Ͽ� ������ �������մϴ�.
	m_QuadTree->Render(m_Frustum, m_Direct3D->GetDeviceContext(), m_TerrainShader);
	/***** ���� ������ : ���� *****/


	/***** �� ������ : ���� *****/	
	m_ModelManager->Render(m_Direct3D->GetDeviceContext(), viewMatrix, projectionMatrix, m_Camera->GetPosition(), deltaTime); // �� �Ŵ����κ��� �𵨵��� �������մϴ�.
	/***** �� ������ : ���� *****/


	/***** �ؽ�Ʈ ������ : ���� *****/
	// ��� 2D �������� �����Ϸ��� Z ���۸� ���ϴ�.
	m_Direct3D->TurnZBufferOff();

	// �ؽ�Ʈ�� �������ϱ� ���� ���� ������ �մϴ�
	m_Direct3D->TurnOnAlphaBlending();

	// �ؽ�Ʈ �Ŵ����κ��� �ؽ�Ʈ���� �������մϴ�.
	m_TextManager->Render(m_Direct3D->GetDeviceContext(), worldMatrix, orthoMatrix);

	// �ؽ�Ʈ�� ������ �� �� ���� ������ �����մϴ�
	m_Direct3D->TurnOffAlphaBlending();

	// ��� 2D �������� �Ϸ�Ǿ����Ƿ� Z ���۸� �ٽ� �ѽʽÿ�.
	m_Direct3D->TurnZBufferOn();
	/***** �ؽ�Ʈ ������ : ���� *****/


	// ������ ������ ȭ�鿡 ����մϴ�
	m_Direct3D->EndScene();

	return true;
}

void RenderingEngine::CaluateAverageDeltaTime(float deltaTime)
{
	/***** ��� ��Ÿ Ÿ�� ���ϴ� ���� : ���� *****/
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
	/***** ��� ��Ÿ Ÿ�� ���ϴ� ���� : ���� *****/


}