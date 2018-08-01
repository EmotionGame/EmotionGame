#include "stdafx.h"
#include <mmsystem.h>
#include "FPS.h"

FPS::FPS()
{
}

FPS::FPS(const FPS& other)
{
}

FPS::~FPS()
{
}

bool FPS::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();

	return true;
}

bool FPS::Frame()
{
	m_count++;

	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;

		m_startTime = timeGetTime();
	}

	return true;
}

int FPS::GetFps()
{
	return m_fps;
}