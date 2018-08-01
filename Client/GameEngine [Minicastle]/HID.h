#pragma once

#include "Mouse_Keyboard.h"

class HID : public AlignedAllocationPolicy<16>
{
public:
	HID();
	HID(const HID& other);
	~HID();

	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Frame();
	
	Mouse_Keyboard* GetMouse_Keyboard();

private:
	HWND m_hwnd;

	Mouse_Keyboard* m_Mouse_Keyboard;
};