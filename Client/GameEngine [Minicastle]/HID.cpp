#include "stdafx.h"
#include "HID.h"

HID::HID()
{
}

HID::HID(const HID& other)
{
}

HID::~HID()
{
}

bool HID::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	m_hwnd = hwnd;

	m_Mouse_Keyboard = new Mouse_Keyboard;
	if (!m_Mouse_Keyboard)
	{
		MessageBox(m_hwnd, L"HID.cpp : m_Mouse_Keyboard = new Mouse_Keyboard;", L"Error", MB_OK);
		return false;
	}

	if (!m_Mouse_Keyboard->Initialize(hinstance, hwnd, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"HID.cpp : m_Mouse_Keyboard->Initialize(hinstance, hwnd, screenWidth, screenHeight)", L"Error", MB_OK);
		return false;
	}

	return true;
}

void HID::Shutdown()
{
	if (m_Mouse_Keyboard)
	{
		m_Mouse_Keyboard->Shutdown();
		delete m_Mouse_Keyboard;
		m_Mouse_Keyboard = nullptr;
	}
}

bool HID::Frame()
{
	if (!m_Mouse_Keyboard->Frame())
	{
		MessageBox(m_hwnd, L"HID.cpp : m_Mouse_Keyboard->Frame()", L"Error", MB_OK);
		return false;
	}

	return true;
}

Mouse_Keyboard* HID::GetMouse_Keyboard()
{
	return m_Mouse_Keyboard;
}