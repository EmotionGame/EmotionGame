#include "stdafx.h"
#include "Mouse_Keyboard.h"

Mouse_Keyboard::Mouse_Keyboard()
{
}

Mouse_Keyboard::Mouse_Keyboard(const Mouse_Keyboard& other)
{
}

Mouse_Keyboard::~Mouse_Keyboard()
{
}

bool Mouse_Keyboard::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	m_hwnd = hwnd;

	// 마우스 커서의 위치 지정에 사용될 화면 크기를 설정합니다.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Direct Input 인터페이스를 초기화 합니다.
	HRESULT result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);", L"Error", MB_OK);
		return false;
	}

	// 키보드의 Direct Input 인터페이스를 생성합니다
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);", L"Error", MB_OK);
		return false;
	}

	// 데이터 형식을 설정하십시오. 이 경우 키보드이므로 사전 정의 된 데이터 형식을 사용할 수 있습니다.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_keyboard->SetDataFormat(&c_dfDIKeyboard);", L"Error", MB_OK);
		return false;
	}

	// 다른 프로그램과 공유하지 않도록 키보드의 협조 수준을 설정합니다
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);", L"Error", MB_OK);
		return false;
	}

	// 키보드를 할당받는다
	result = m_keyboard->Acquire();
	if (FAILED(result))
	{
		//MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_keyboard->Acquire();", L"Error", MB_OK);
		//return false;
	}

	// 마우스 Direct Input 인터페이스를 생성합니다.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);", L"Error", MB_OK);
		return false;
	}

	// 미리 정의 된 마우스 데이터 형식을 사용하여 마우스의 데이터 형식을 설정합니다.
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_mouse->SetDataFormat(&c_dfDIMouse);", L"Error", MB_OK);
		return false;
	}

	// 다른 프로그램과 공유 할 수 있도록 마우스의 협력 수준을 설정합니다. DISCL_NONEXCLUSIVE -> DISCL_EXCLUSIVE로 못하게 변경
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);", L"Error", MB_OK);
		return false;
	}

	// 마우스를 할당받는다
	result = m_mouse->Acquire();
	if (FAILED(result))
	{
		//MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : m_mouse->Acquire();", L"Error", MB_OK);
		//return false;
	}

	return true;
}

void Mouse_Keyboard::Shutdown()
{
	// 마우스를 반환합니다.
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = nullptr;
	}

	// 키보드를 반환합니다.
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = nullptr;
	}

	// IDirectInput8 객체를 반환합니다.
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = nullptr;
	}
}

bool Mouse_Keyboard::Frame()
{
	// 키보드의 현재 상태를 읽는다.
	if (!ReadKeyboard())
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : ReadKeyboard()", L"Error", MB_OK);
		return false;
	}

	// 마우스의 현재 상태를 읽는다.
	if (!ReadMouse())
	{
		MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : ReadMouse()", L"Error", MB_OK);
		return false;
	}



	// 키보드와 마우스의 변경상태를 처리합니다.
	ProcessInput();

	return true;
}

bool Mouse_Keyboard::ReadKeyboard()
{
	// 키보드 디바이스를 얻는다.
	HRESULT result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result))
	{
		// 키보드가 포커스를 잃었거나 획득되지 않은 경우 컨트롤을 다시 가져 온다
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : (result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

bool Mouse_Keyboard::ReadMouse()
{
	// 마우스 디바이스를 얻는다.
	HRESULT result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		// 마우스가 포커스를 잃었거나 획득되지 않은 경우 컨트롤을 다시 가져 온다
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			MessageBox(m_hwnd, L"Mouse_Keyboard.cpp : (result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)", L"Error", MB_OK);
			return false;
		}
	}

	return true;
}

void Mouse_Keyboard::ProcessInput()
{
	/*
	m_mouseState.lX : X축 변화량
	m_mouseState.lY : Y축 변화량
	m_mouseState.lZ : 휠 변화량
	m_mouseState.rgbButtons[0] : 왼쪽 버튼
	m_mouseState.rgbButtons[1] : 오른쪽 버튼
	m_mouseState.rgbButtons[2] : 휠 버튼
	*/

	// 프레임 동안 마우스 위치의 변경을 기반으로 마우스 커서의 위치를 ​​업데이트 합니다.
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;
	m_deltaMouseX = m_mouseState.lX;
	m_deltaMouseY = m_mouseState.lY;

	if (m_deltaMouseX)
		m_savedDeltaMouseX = m_deltaMouseX;
	if (m_deltaMouseY)
		m_savedDeltaMouseY = m_deltaMouseY;


	// 마우스 위치가 화면 너비 또는 높이를 초과하지 않는지 확인한다.
	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth) { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }
}

bool Mouse_Keyboard::IsEscapePressed()
{
	// escape 키가 현재 눌려지고 있는지 bit값을 계산하여 확인한다.
	if (m_keyboardState[DIK_ESCAPE] & 0x80)
	{
		return true;
	}

	return false;
}

void Mouse_Keyboard::GetMouseLocation(int& mouseX, int& mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
}

bool Mouse_Keyboard::IsKeyDown(int key) {
	if (m_keyboardState[key] & 0x80)
	{
		return true;
	}
	return false;
}

bool Mouse_Keyboard::IsKeyRelease(int key) {
	if (IsKeyDown(key)) {
		if (m_keyboardPressed[key] == false) {
			m_keyboardPressed[key] = true;
			m_keyboardReleased[key] = false;
		}
	}
	else {
		if (m_keyboardPressed[key] == true) {
			m_keyboardReleased[key] = true;
			m_keyboardPressed[key] = false;
		}
	}

	if (m_keyboardReleased[key]) {
		m_keyboardReleased[key] = false;
		m_keyboardPressed[key] = false;

		return true;
	}
	else
		return false;
}

bool Mouse_Keyboard::IsMouseLeftButtonDown() {
	if (m_mouseState.rgbButtons[0] & 0x80) {
		return true;
	}
	return false;
}

bool Mouse_Keyboard::IsMouseRightButtonDown() {
	if (m_mouseState.rgbButtons[1] & 0x80) {
		return true;
	}
	return false;
}

bool Mouse_Keyboard::IsMouseWheelButtonDown() {
	if (m_mouseState.rgbButtons[2] & 0x80) {
		return true;
	}
	return false;
}

void Mouse_Keyboard::GetDeltaMouse(int& deltaMouseX, int& deltaMouseY)
{
	deltaMouseX = m_savedDeltaMouseX;
	deltaMouseY = m_savedDeltaMouseY;

	m_savedDeltaMouseX = 0;
	m_savedDeltaMouseY = 0;
}