#include "stdafx.h"
#include "HID.h"
#include "Text.h"
#include "TextManager.h"

TextManager::TextManager()
{
}

TextManager::TextManager(const TextManager& other)
{
}

TextManager::~TextManager()
{
}

bool TextManager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, HWND hwnd, int screenWidth, int screenHeight, XMMATRIX baseViewMatrix)
{
#ifdef _DEBUG
	printf("Start >> TextManager.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;
	
	/***** 정보 : 시작 *****/
	// FPS
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/40pt_Bold.txt", L"Data/Font/40pt_Bold.dds", "  ", 0, -10, 0.0f, 1.0f, 0.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : FPS->Initialize", L"Error", MB_OK);
		return false;
	}

	// CPU
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 40, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : CPU->Initialize", L"Error", MB_OK);
		return false;
	}

	// DeltaTime
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 70, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : DeltaTime->Initialize", L"Error", MB_OK);
		return false;
	}

	// AverageDeltaTime
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 100, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : AverageDeltaTime->Initialize", L"Error", MB_OK);
		return false;
	}

	// PlayerID
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 130, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : PlayerID->Initialize", L"Error", MB_OK);
		return false;
	}

	// TerrainPolygonCount
	m_InfoTexts->push_back(new Text);
	if (!m_InfoTexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_InfoTexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 160, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : PlayerID->Initialize", L"Error", MB_OK);
		return false;
	}
	/***** 정보 : 종료 *****/

	/***** UI : 시작 *****/
	// HP
	m_UITexts->push_back(new Text);
	if (!m_UITexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_UITexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/40pt_Bold.txt", L"Data/Font/40pt_Bold.dds", "  ", 50, screenHeight - 100, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : HP->Initialize", L"Error", MB_OK);
		return false;
	}

	// Speed
	m_UITexts->push_back(new Text);
	if (!m_UITexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_UITexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", screenWidth - 150, 10, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : Speed->Initialize", L"Error", MB_OK);
		return false;
	}

	// Emotion
	m_UITexts->push_back(new Text);
	if (!m_UITexts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_UITexts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/40pt_Bold.txt", L"Data/Font/40pt_Bold.dds", "  ", screenWidth - 400, screenHeight - 100, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : Emotion->Initialize", L"Error", MB_OK);
		return false;
	}
	
	/***** UI : 종료 *****/

#ifdef _DEBUG
	printf("Success >> TextManager.cpp : Initialize()\n");
#endif

	return true;
}

void TextManager::Shutdown()
{
	/***** UI : 시작 *****/
	for (auto iter = m_UITexts->begin(); iter != m_UITexts->end(); iter++)
	{
		(*iter)->Shutdown();
		delete *iter;
		*iter = nullptr;
	}
	m_UITexts->clear();
	delete m_UITexts;
	m_UITexts = nullptr;
	/***** UI : 종료 *****/

	/***** 정보 : 시작 *****/
	for (auto iter = m_InfoTexts->begin(); iter != m_InfoTexts->end(); iter++)
	{
		(*iter)->Shutdown();
		delete *iter;
		*iter = nullptr;
	}
	m_InfoTexts->clear();
	delete m_InfoTexts;
	m_InfoTexts = nullptr;
	/***** 정보 : 종료 *****/
}

bool TextManager::Frame(ID3D11DeviceContext* pDeviceContext, HID* pHID, int cputPercentage, int fps, float deltaTime, float averageDeltaTime, int playerID, int terrainPolygonCount,
	int hp, float speed, int emotion[4])
{
	/***** 정보 : 시작 *****/
	if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F1))
	{
		m_InfoRenderFlag = m_InfoRenderFlag ? false : true;
	}

	// FPS
	if (!m_InfoTexts->at(0)->SetSentenceWithINT("", fps, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(0)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// CPU
	if (!m_InfoTexts->at(1)->SetSentenceWithINT("CPU: ", cputPercentage, "%", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(1)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// DeltaTime
	if (!m_InfoTexts->at(2)->SetSentenceWithFLOAT("DeltaTime: ", deltaTime, "ms", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(2)->SetSentenceWithFLOAT", L"Error", MB_OK);
		return false;
	}

	// AverageDeltaTime
	if (!m_InfoTexts->at(3)->SetSentenceWithFLOAT("AverageDeltaTime: ", averageDeltaTime, "ms", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(3)->SetSentenceWithFLOAT", L"Error", MB_OK);
		return false;
	}

	// PlayerID
	if (!m_InfoTexts->at(4)->SetSentenceWithINT("PlayerID: ", playerID, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(4)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// TerrainPolygonCount
	if (!m_InfoTexts->at(5)->SetSentenceWithINT("TerrainPolygonCount: ", terrainPolygonCount, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts.at(5)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}
	/***** 정보 : 종료 *****/

	/***** UI : 시작 *****/
	// HP
	if (!m_UITexts->at(0)->SetSentenceWithINT("HP: ", hp, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.at(0)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}
	// Speed
	if (!m_UITexts->at(1)->SetSentenceWithFLOAT("Speed: ", speed, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.at(1)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// Emotion
	if (!m_UITexts->at(2)->SetSentenceWithINT4("Emotion: ", emotion, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts.at(2 + i)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}
	/***** UI : 종료 *****/

	return true;
}

bool TextManager::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	/***** 정보 : 시작 *****/
	if (m_InfoRenderFlag)
	{
		auto last = m_InfoTexts->end();
		for (auto first = m_InfoTexts->begin(); first != last; first++)
		{
			if (!(*first)->Render(pDeviceContext, worldMatrix, orthoMatrix))
			{
				MessageBox(m_hwnd, L"TextManager.cpp : m_InfoTexts->Render(deviceContext, worldMatrix, orthoMatrix)", L"Error", MB_OK);
				return false;
			}
		}
	}
	/***** 정보 : 종료 *****/

	/***** UI : 시작 *****/
	auto last = m_UITexts->end();
	for (auto first = m_UITexts->begin(); first != last; first++)
	{
		if (!(*first)->Render(pDeviceContext, worldMatrix, orthoMatrix))
		{
			MessageBox(m_hwnd, L"TextManager.cpp : m_UITexts->Render(deviceContext, worldMatrix, orthoMatrix)", L"Error", MB_OK);
			return false;
		}
	}
	/***** UI : 종료 *****/

	return true;
}


