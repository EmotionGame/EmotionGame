#include "stdafx.h"
#include "FBX.h"
#include "KSM.h"
#include "DroppedFileInfo.h"

DroppedFileInfo::DroppedFileInfo()
{
}

DroppedFileInfo::~DroppedFileInfo()
{
}

void DroppedFileInfo::Initialize(HWND hwnd)
{
	m_hwnd = hwnd;

	m_KSM = new KSM;

	(HANDLE)_beginthreadex(NULL, 0, Thread, (LPVOID)this, 0, NULL);
}

void DroppedFileInfo::OnDropFiles(HANDLE hDropInfo)
{
	/***** 잠금 *****/
	m_PathQueueMutex.lock();

	// 전체 드롭된 항목의 개수를 얻는다.
	int count = DragQueryFileA((HDROP)hDropInfo, 0xFFFFFFFF, NULL, 0);
	
	// 항목 개수를 저장하기 위한 변수
	int dir_count = 0, file_count = 0;

	for (int i = 0; i < count; i++) {
		// 큐에 Push했을 때 복사가 아닌 주소값을 저장합니다.
		// 그런데, tempPath는 지역변수이므로 이 함수가 끝나면 사라집니다.
		// 따라서, 동적으로 할당한 뒤에 사용이 끝나면 해제해주는 작업을 해야합니다.
		char* tempPath = new char[MAX_PATH];

		// 드랍된 항목중에 i 번째 있는 항목의 이름을 얻는다.
		DragQueryFileA((HDROP)hDropInfo, i, tempPath, MAX_PATH);

		// 드롭된 항목이 파일인지 폴더인지 체크해서 개수를 증가시킨다.
		if ((GetFileAttributesA(tempPath) & FILE_ATTRIBUTE_DIRECTORY))
			dir_count++;
		else 
			file_count++;

		// 확장자가 .fbx인지 확인
		if (Last4strcmp(tempPath, ".fbx") || Last4strcmp(tempPath, ".FBX"))
		{
			for (unsigned int j = 0; j < strlen(tempPath); j++)
			{
				if (tempPath[j] == '\\')
					tempPath[j] = '/';
			}

			m_PathQueue->push(tempPath);
			printf("m_PathQueue.push(temp_path);\n");
			printf("front() : %s\n", m_PathQueue->front());
		}
	}

	m_PathQueueMutex.unlock();
	/***** 해제 *****/
}

unsigned int __stdcall DroppedFileInfo::Thread(void* p)
{
	static_cast<DroppedFileInfo*>(p)->_Thread();

	return true;
}

UINT WINAPI DroppedFileInfo::_Thread()
{
	while (true) {
		m_PathQueueMutex.lock();
		if (!m_PathQueue->empty())
		{
			m_FBX = new FBX;
			printf("front() : %s\n", m_PathQueue->front());
			printf("Start >> g_FBX->Initialize\n");

			if (!m_FBX->Initialize(m_hwnd, m_PathQueue->front())) // 에러 발생
			{
				printf("Fail >> g_FBX->Initialize\n");
			}
			else // 정상적으로 초기화 완료
			{
				printf("End >> g_FBX->Initialize\n");

				m_KSM->SaveFBX2KSM(m_FBX, m_PathQueue->front());

				printf("Start >> g_FBX->Shutdown\n");
				m_FBX->Shutdown();
				printf("End >> g_FBX->Shutdown\n");
			}

			delete[] m_PathQueue->front();
			m_PathQueue->pop();
			printf("m_PathQueue.pop();\n");

			delete m_FBX;
			m_FBX = nullptr;
		}
		m_PathQueueMutex.unlock();
	}

	return true;
}


// 확장자 비교용 함수
bool DroppedFileInfo::Last4strcmp(const char* pFileName, const char* pLast4FileName) {
	int filenameLen = strlen(pFileName);
	char last[5];

	last[0] = pFileName[filenameLen - 4];
	last[1] = pFileName[filenameLen - 3];
	last[2] = pFileName[filenameLen - 2];
	last[3] = pFileName[filenameLen - 1];
	last[4] = '\0';

	if (strcmp(last, pLast4FileName) == 0)
		return true;

	return false;
}