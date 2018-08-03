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
	/***** ��� *****/
	m_PathQueueMutex.lock();

	// ��ü ��ӵ� �׸��� ������ ��´�.
	int count = DragQueryFileA((HDROP)hDropInfo, 0xFFFFFFFF, NULL, 0);
	
	// �׸� ������ �����ϱ� ���� ����
	int dir_count = 0, file_count = 0;

	for (int i = 0; i < count; i++) {
		// ť�� Push���� �� ���簡 �ƴ� �ּҰ��� �����մϴ�.
		// �׷���, tempPath�� ���������̹Ƿ� �� �Լ��� ������ ������ϴ�.
		// ����, �������� �Ҵ��� �ڿ� ����� ������ �������ִ� �۾��� �ؾ��մϴ�.
		char* tempPath = new char[MAX_PATH];

		// ����� �׸��߿� i ��° �ִ� �׸��� �̸��� ��´�.
		DragQueryFileA((HDROP)hDropInfo, i, tempPath, MAX_PATH);

		// ��ӵ� �׸��� �������� �������� üũ�ؼ� ������ ������Ų��.
		if ((GetFileAttributesA(tempPath) & FILE_ATTRIBUTE_DIRECTORY))
			dir_count++;
		else 
			file_count++;

		// Ȯ���ڰ� .fbx���� Ȯ��
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
	/***** ���� *****/
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

			if (!m_FBX->Initialize(m_hwnd, m_PathQueue->front())) // ���� �߻�
			{
				printf("Fail >> g_FBX->Initialize\n");
			}
			else // ���������� �ʱ�ȭ �Ϸ�
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


// Ȯ���� �񱳿� �Լ�
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