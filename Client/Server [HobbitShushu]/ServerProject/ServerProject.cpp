// ServerProject.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#include <iostream>
#include "IOCPServer.h"
#include "UserManager.h"

using namespace std;

int main()
{
	IOCPServer* s = new IOCPServer();
	s->init();
	s->setPool();
	Sleep(INFINITE);
	delete s;

	return 0;
}