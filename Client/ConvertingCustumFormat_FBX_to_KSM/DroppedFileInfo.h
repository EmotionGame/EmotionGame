#pragma once

class FBX;
class KSM;

class DroppedFileInfo
{
public:
	DroppedFileInfo();
	~DroppedFileInfo();

	void Initialize(HWND hwnd);
	void OnDropFiles(HANDLE hDropInfo);

	unsigned int static __stdcall Thread(void* p);
	UINT WINAPI _Thread();

	bool Last4strcmp(const char* pFileName, const char* pLast4FileName);

	HWND m_hwnd;

	std::queue<char*>* m_PathQueue = new std::queue<char*>;
	std::mutex m_PathQueueMutex;

	FBX* m_FBX = nullptr;
	KSM* m_KSM = nullptr;
};