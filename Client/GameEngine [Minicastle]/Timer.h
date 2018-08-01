#pragma once

class Timer : public AlignedAllocationPolicy<16>
{
public:
	Timer();
	Timer(const Timer& other);
	~Timer();

	bool Initialize();
	bool Frame();

	float GetTime();

private:
	INT64 m_frequency = 0;
	float m_ticksPerMs = 0.0f;
	INT64 m_startTime = 0;
	float m_frameTime = 0.0f;

};