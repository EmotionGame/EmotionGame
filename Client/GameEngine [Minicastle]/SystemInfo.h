#pragma once

class CPU;
class Timer;

class SystemInfo : public AlignedAllocationPolicy<16>
{
public:
	SystemInfo();
	SystemInfo(const SystemInfo& other);
	~SystemInfo();

	bool Initialize(HWND hwnd);
	void Shutdown();
	bool Frame(int& m_CpuPercentage, float& m_DeltaTime);

	int GetCpuPercentage();
	float GetTime();

private:
	HWND m_hwnd;

	CPU* m_CPU;
	Timer* m_Timer;
};