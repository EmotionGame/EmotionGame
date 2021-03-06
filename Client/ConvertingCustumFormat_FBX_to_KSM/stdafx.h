// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#include "Resource.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ 런타임 헤더 파일입니다.
#include <vector>
#include <list>
#include <queue>
#include <unordered_map>
#include <utility>
#include <mutex>
#include <thread>
#include <process.h>

#include <shellapi.h>

// FBX SDK 헤더 파일입니다.
#include <fbxsdk.h>

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
using namespace DirectX;

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
