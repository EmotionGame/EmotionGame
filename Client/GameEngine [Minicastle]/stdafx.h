/*****  제작자: 김성민	(KimSungMin)							*****/
/*****  국적: 대한민국(Korea)									*****/
/*****	학력: 홍익대학교 세종캠퍼스 게임소프트웨어 3학년 재학중		*****/
/*****	동아리: Exdio(게임 제작 소모임)							*****/
/*****	Phone: 010-8865-0312								*****/
/*****	GitHub: Minicastle									*****/
/*****  Blog: blog.naver.com/bloodxsecter					*****/
/*****  인턴 경험: 블루홀 클라이언트 프로그래머 [180625-180817]	*****/
/*****  라이센스(License): 없음(Free)							*****/
/*****  Thank You!											*****/

// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

#define _WIN32_WINNT 0x0600				// <== 추가

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
//#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <process.h> // _beginthread함수용

// C++ 런타임 헤더 파일입니다.
#include <vector>
#include <list>
#include <queue>
#include <unordered_map>
#include <utility>
#include <string>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono> // NetworkEngine
#include <mutex> // c++11에서 추가된 std::mutex를 사용하기 위해
#include <random> // 난수 생성

// 네트워크 헤더 파일입니다.
#include <winsock2.h>
#include <WS2tcpip.h> // inet_addr 대신 inet_pton을 사용하기 위해
#pragma comment(lib, "ws2_32.lib") // winsock2 사용시 에러 발생 해결 : error LNK2019 : __imp__closesocket@4 외부 기호(참조 위치: _main 함수)에서 확인하지 못했습니다. 추가 종속성에 포함해도 OK

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "DxDefine.h"

// FBX SDK 헤더 파일입니다.
#include <fbxsdk.h>

// 구조체 선언
struct VertexType // 나중에 ModelVertexType으로 변경
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
	unsigned int materialIndex;
};
struct AnimationVertexType // 나중에 ModelAnimationVertexType으로 변경
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
	unsigned int materialIndex;
	unsigned int blendingIndex[4];
	float blendingWeight[4];
};

/***** ModelManager : 시작 *****/
#define MATERIAL_SIZE 32
#define BONE_FINAL_TRANSFORM_SIZE 256
struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

struct CameraBufferType
{
	XMFLOAT3 cameraPosition;
	float padding; // 바이트 수를 맞추기 위한 패딩
};

struct LightBufferType
{
	XMFLOAT4 ambientColor[MATERIAL_SIZE];
	XMFLOAT4 diffuseColor[MATERIAL_SIZE];
	XMFLOAT4 specularColor[MATERIAL_SIZE];
	XMFLOAT4 specularPower[MATERIAL_SIZE]; // x만 사용. yzw는 패딩용...
	XMFLOAT3 lightDirection;
	float padding;  // 바이트 수를 맞추기 위한 패딩
};
/***** ModelManager : 시작 *****/

/***** NetworkEngine : 시작 *****/
#define QUEUE_LIMIT_SIZE 5000


struct UserPacket // type = 30(자신) or 31(타인들)
{
	int type = 30;
	int id = 0;
	int hp = 0;
	int speed = 0;
	int emotion[4] = { 0, 0, 0, 0 };
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };
};

struct ActionPacket // type = 11
{
	int type = 11;
	int id = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };
};

struct EventPacket // type = 21
{
	int type = 21;
	int id = 0;
	float position[3] = { 0.0f, 0.0f , 0.0f };
	bool state = false;
};
/***** NetworkEngine : 종료 *****/