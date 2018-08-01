#pragma once

/////////////
// LINKING //
/////////////
#pragma comment(lib, "winmm.lib")


class FPS : public AlignedAllocationPolicy<16>
{
public:
	FPS();
	FPS(const FPS& other);
	~FPS();

	bool Initialize();
	bool Frame();
	int GetFps();

private:
	int m_fps, m_count;
	unsigned long m_startTime;
};