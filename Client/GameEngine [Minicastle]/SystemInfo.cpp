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

	// CPU ��ü�� �����մϴ�.
	m_CPU = new CPU;
	if (!m_CPU)
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_CPU = new CPU;", L"Error", MB_OK);
		return false;
	}

	// CPU ��ü�� �ʱ�ȭ�մϴ�.
	if (!m_CPU->Initialize())
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_CPU->Initialize()", L"Error", MB_OK);
		return false;
	}

	// Timer ��ü�� �����մϴ�.
	m_Timer = new Timer;
	if (!m_Timer)
	{
		MessageBox(m_hwnd, L"SystemInfo.cpp : m_Timer = new Timer", L"Error", MB_OK);
		return false;
	}

	// Timer ��ü�� �ʱ�ȭ�մϴ�.
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
	// Timer ��ü�� �����մϴ�.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}

	// CPU ��ü�� �����մϴ�.
	if (m_CPU)
	{
		m_CPU->Shutdown();
		delete m_CPU;
		m_CPU = nullptr;
	}
}

bool SystemInfo::Frame(int& m_CpuPercentage, float& m_DeltaTime)
{
	// �ý��� ��踦 ������Ʈ �մϴ�
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