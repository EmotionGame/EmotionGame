#pragma once

// 에러 : dinput.h: DIRECTINPUT_VERSION undefined. Defaulting to version 0x0800
#define DIRECTINPUT_VERSION 0x0800

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib") // LNK2001: IDD_IDirectInput8W 외부 기호를 확인할 수 없습니다. _GUID_SysKeyboard 등등 에러 때문에 추가

//////////////
// INCLUDES //
//////////////
#include <d3d11_1.h>
#include <dinput.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "DDSTextureLoader.h"	// DDS 파일 처리
#include "WICTextureLoader.h"	// WIC (BMP, JPEG, PNG, TIFF, GIF, etc..) 파일 처리
using namespace DirectX;


///////////////////////////
//  warning C4316 처리용  //
///////////////////////////
#include "AlignedAllocationPolicy.h"

/* 매크로
XM_PI = 3.141592654f;
float XMConvertToRadians(float fDegrees) { return fDegrees * (XM_PI / 180.0f); }
float XMConvertToDegrees(float fRadians) { return fRadians * (180.0f / XM_PI); }
*/
#define XM_RADIAN XM_PI / 180.0f