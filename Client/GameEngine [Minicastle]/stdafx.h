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
#include <time.h>

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

enum CollisionType
{
	AxisAlignedBoundingBox,
	OrientedBoundingBox
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

struct ActionPacket // type = 11
{
	int type = 11;
	int id = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };
	float acceleration[3] = { 0.0f, 0.0f, 0.0f };
};

struct EventPacket // type = 21
{
	int type = 21;
	int id = 0;
	/* id
	1 : 고양이	: 생성
	2 : 강아지	: 생성
	3 : 모닥불	: 고정
	4 : 지압 자갈	: 고정
	5 : 피격		: 발생
	6 : 모래지옥	: 고정
	7 : 번개		: 생성
	8 : 순금 용	: 생성
	*/
	float position[3] = { 0.0f, 0.0f , 0.0f };
	bool state = false;
};

struct EventAcquirePacket
{
	int type = 22;
	int eventId = 0;
	int playerId = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
};

struct UserPacket // type = 30(초기화), type = 31(값 변경)
{
	int type = 30;
	int id = 0;
	int hp = 100;
	float speed = 10.0f;
	int emotion[4] = { 0, 0, 0, 0 };
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };
	float scale[3] = { 0.0f, 0.0f, 0.0f };
	float acceleration[3] = { 0.0f, 0.0f, 0.0f };
};

struct ObejctPacket
{
	int type = 40;
	int id = 0;
	int hp = 40;
	int emotion[4] = { 0, 0, 0, 0 };
	float position[3] = { 0.0f, 0.0f, 0.0f };
	bool state = true;
};

struct MonsterPacket
{
	int type = 50;
	float speed = 9.0f;
	int emotion[4] = { 0, 0, 0, 0 };
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float rotation[3] = { 0.0f, 0.0f, 0.0f };
	float scale[3] = { 0.0f, 0.0f, 0.0f };
	int damage = 20;
};

struct MonsterAttackPacket
{
	int type = 51;
	int playerId = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	bool collision = false;
};

struct Player2Player
{
	int type = 60;
	int player1Id = 0; // 감정을 전달하는 플레이어
	int player2Id = 0; // 감정을 전달 당하는 플레이어
	int emotion[4] = { 0, 0, 0, 0 }; // 전달된 감정 수치 { 0, 3, 0, 0} 식으로
};

struct Player2Monster
{
	int type = 61;
	int emotion[4] = { 0, 0, 0, 0 };
};

struct Player2Object
{
	int type = 62;
	int objectId = 0; // 감정이 상승되는 오브젝트
	int emotion[4] = { 0, 0, 0, 0 }; // 상승된 감정 수치
};

struct GameOverPacket
{
	int type = 100;
	bool winner = false;
};
/***** NetworkEngine : 종료 *****/