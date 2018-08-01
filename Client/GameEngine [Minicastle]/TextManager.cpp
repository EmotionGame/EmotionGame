#include "stdafx.h"
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
	
	// FPS
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/40pt_Bold.txt", L"Data/Font/40pt_Bold.dds", "  ", 0, -10, 0.0f, 1.0f, 0.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : FPS->Initialize", L"Error", MB_OK);
		return false;
	}

	// CPU
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 40, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : CPU->Initialize", L"Error", MB_OK);
		return false;
	}

	// DeltaTime
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 70, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : DeltaTime->Initialize", L"Error", MB_OK);
		return false;
	}

	// AverageDeltaTime
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 100, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : AverageDeltaTime->Initialize", L"Error", MB_OK);
		return false;
	}

	// PlayerID
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 130, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : PlayerID->Initialize", L"Error", MB_OK);
		return false;
	}

	// TerrainPolygonCount
	m_Texts->push_back(new Text);
	if (!m_Texts->back())
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.back()", L"Error", MB_OK);
		return false;
	}
	if (!m_Texts->back()->Initialize(pDevice, pDeviceContext, hwnd, screenWidth, screenHeight, baseViewMatrix,
		"Data/Font/25pt.txt", L"Data/Font/25pt.dds", "  ", 40, 190, 1.0f, 1.0f, 1.0f))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : PlayerID->Initialize", L"Error", MB_OK);
		return false;
	}

#ifdef _DEBUG
	printf("Success >> TextManager.cpp : Initialize()\n");
#endif

	return true;
}

void TextManager::Shutdown()
{
	for (auto iter = m_Texts->begin(); iter != m_Texts->end(); iter++)
	{
		(*iter)->Shutdown();
		delete *iter;
		*iter = nullptr;
	}
	m_Texts->clear();
	delete m_Texts;
	m_Texts = nullptr;
}

bool TextManager::Frame(ID3D11DeviceContext* pDeviceContext, int cputPercentage, int fps, float deltaTime, float averageDeltaTime, int playerID, int terrainPolygonCount)
{
	// FPS
	if (!m_Texts->at(0)->SetSentenceWithINT("", fps, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(0)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// CPU
	if (!m_Texts->at(1)->SetSentenceWithINT("CPU: ", cputPercentage, "%", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(1)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// DeltaTime
	if (!m_Texts->at(2)->SetSentenceWithFLOAT("DeltaTime: ", deltaTime, "ms", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(2)->SetSentenceWithFLOAT", L"Error", MB_OK);
		return false;
	}

	// AverageDeltaTime
	if (!m_Texts->at(3)->SetSentenceWithFLOAT("AverageDeltaTime: ", averageDeltaTime, "ms", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(3)->SetSentenceWithFLOAT", L"Error", MB_OK);
		return false;
	}

	// PlayerID
	if (!m_Texts->at(4)->SetSentenceWithINT("PlayerID: ", playerID, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(4)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	// TerrainPolygonCount
	if (!m_Texts->at(5)->SetSentenceWithINT("TerrainPolygonCount: ", terrainPolygonCount, "", pDeviceContext))
	{
		MessageBox(m_hwnd, L"TextManager.cpp : m_Texts.at(5)->SetSentenceWithINT", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool TextManager::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	auto last = m_Texts->end();
	for (auto first = m_Texts->begin(); first != last; first++)
	{
		if (!(*first)->Render(pDeviceContext, worldMatrix, orthoMatrix))
		{
			MessageBox(m_hwnd, L"TextManager.cpp : (*first)->Render(deviceContext, worldMatrix, orthoMatrix)", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}


