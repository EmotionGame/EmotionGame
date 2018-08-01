#include "stdafx.h"
#include "CPU.h"

CPU::CPU()
{
}

CPU::CPU(const CPU& other)
{
}

CPU::~CPU()
{
}

bool CPU::Initialize()
{
	// CPU ����� �����ϴ� ���� ��ü�� ����ϴ�.
	PDH_STATUS status = PdhOpenQuery(NULL, 0, &m_queryHandle);
	if (status != ERROR_SUCCESS)
	{
		m_canReadCpu = false;
	}

	// �ý����� ��� CPU�� �����ϵ��� ���� ��ü�� �����մϴ�.
	status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
	if (status != ERROR_SUCCESS)
	{
		m_canReadCpu = false;
	}

	m_lastSampleTime = GetTickCount();

	m_cpuUsage = 0;

	return true;
}

void CPU::Shutdown()
{
	if (m_canReadCpu)
	{
		PdhCloseQuery(m_queryHandle);
	}
}

bool CPU::Frame()
{
	PDH_FMT_COUNTERVALUE value;

	if (m_canReadCpu)
	{
		if ((m_lastSampleTime + 1000) < GetTickCount())
		{
			m_lastSampleTime = GetTickCount();

			PdhCollectQueryData(m_queryHandle);

			PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);

			m_cpuUsage = value.longValue;
		}
	}

	return true;
}

int CPU::GetCpuPercentage()
{
	int usage = 0;

	if (m_canReadCpu)
	{
		usage = (int)m_cpuUsage;
	}

	return usage;
}