#include "stdafx.h"
#include "CPU.h"
#include "Timer.h"
#include "SystemInfo.h"

SystemInfo::SystemInfo()
{
}

SystemInfo::SystemInfo(const SystemInfo& other)
{
}

SystemInfo::~SystemInfo()
{
}

bool SystemInfo::Initialize(HWND hwnd)
{
#ifdef _DEBUG
	printf("Start >> SystemInfo.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;

	// CPU 객체를 생성합니다.
	m_CPU = new CPU;
	if (!m_CPU)
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_CPU = new CPU;", L"Error", MB_OK);
		return false;
	}

	// CPU 객체를 초기화합니다.
	if (!m_CPU->Initialize())
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_CPU->Initialize()", L"Error", MB_OK);
		return false;
	}

	// Timer 객체를 생성합니다.
	m_Timer = new Timer;
	if (!m_Timer)
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_Timer = new Timer", L"Error", MB_OK);
		return false;
	}

	// Timer 객체를 초기화합니다.
	if (!m_Timer->Initialize())
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_Timer->Initialize()", L"Error", MB_OK);
		return false;
	}

#ifdef _DEBUG
	printf("Success >> SystemInfo.cpp : Initialize()\n");
#endif

	return true;
}

void SystemInfo::Shutdown()
{
	// Timer 객체를 해제합니다.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}

	// CPU 객체를 해제합니다.
	if (m_CPU)
	{
		m_CPU->Shutdown();
		delete m_CPU;
		m_CPU = nullptr;
	}
}

bool SystemInfo::Frame(int& m_CpuPercentage, float& m_DeltaTime)
{
	// 시스템 통계를 업데이트 합니다
	if (!m_CPU->Frame())
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_CPU->Frame()", L"Error", MB_OK);
		return false;
	}

	if (!m_Timer->Frame())
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_Timer->Frame()", L"Error", MB_OK);
		return false;
	}

	m_CpuPercentage = GetCpuPercentage();
	m_DeltaTime = GetTime();

	return true;
}

int SystemInfo::GetCpuPercentage()
{
	return m_CPU->GetCpuPercentage();
}

float SystemInfo::GetTime()
{
	return m_Timer->GetTime();
}