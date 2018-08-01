#pragma once

/////////////
// LINKING //
/////////////
#pragma comment(lib, "pdh.lib")
#include <pdh.h>

class CPU : public AlignedAllocationPolicy<16>
{
public:
	CPU();
	CPU(const CPU& other);
	~CPU();

	bool Initialize();
	void Shutdown();
	bool Frame();

	int GetCpuPercentage();

private:
	bool m_canReadCpu = true;
	HQUERY m_queryHandle;
	HCOUNTER m_counterHandle;
	unsigned long m_lastSampleTime = 0;
	long m_cpuUsage = 0;
};