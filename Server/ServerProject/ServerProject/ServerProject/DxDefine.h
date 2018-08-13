#pragma once

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib") // LNK2001: IDD_IDirectInput8W 외부 기호를 확인할 수 없습니다. _GUID_SysKeyboard 등등 에러 때문에 추가

//////////////
// INCLUDES //
//////////////
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
//using namespace DirectX;
