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

#include "stdafx.h"
#include "GameEngine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	// GameEngine 객체 생성
	GameEngine* GE = new GameEngine;
	if (!GE)
	{
		return -1;
	}

	// GameEngine 객체 초기화 및 실행
	if (GE->Initialize())
	{
		GE->Run();
	}

	// GameEngine 객체 종료 및 메모리 반환
	GE->Shutdown();
	delete GE;
	GE = nullptr;

	return 0;
}