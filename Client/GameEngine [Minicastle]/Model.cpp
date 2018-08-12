#include "stdafx.h"
#include "Direct3D.h"
#include "HID.h"
#include "Model.h"

#include <fstream>

/********** 파일 로딩 : 시작 **********/
bool Model::LoadFBX2KSM(char* pFileName)
{
	// 초기 정보 획득
	LoadFBXInfo(pFileName);
	LoadHasAnimation(pFileName);
	LoadGlobalOffPosition(pFileName);

	// 각 메시마다 수행
	for (unsigned int meshCount = 0; meshCount < m_Info.meshCount; meshCount++)
	{
		if (m_HasAnimation.at(meshCount))
		{
			LoadVertexAnim(pFileName, meshCount);

			for (unsigned int animStackCount = 0; animStackCount < m_Info.animStackCount; animStackCount++)
			{
				LoadFinalTransform(pFileName, meshCount, animStackCount);
			}
		}
		else
		{
			LoadVertex(pFileName, meshCount);
		}

		LoadPolygon(pFileName, meshCount);

		// 머터리얼이 존재하면
		if (m_Info.hasMaterial)
		{
			LoadMaterial(pFileName, meshCount);
		}
	}

	// 디버그에서 값 확인용
	m_Info;
	m_HasAnimation;
	m_Vertex;
	m_VertexAnim;
	m_polygon;
	m_Material;
	m_GlobalOffPosition;
	m_Animations;

	return true;
}

bool Model::LoadFBXInfo(char* pFileName)
{
	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s", copy, "_info.ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	Info* forRead;
	char buf[BUFFER_SIZE];

	if (fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB) <= 0)
	{
		return false;
	}

	char* readBuf = new char[sizeof(Info)];
	memcpy(readBuf, buf, sizeof(Info));
	forRead = reinterpret_cast<Info*>(readBuf);

	m_Info = *forRead;

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/

	return true;
}

bool Model::LoadHasAnimation(char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_HasAnimation_", ".ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	bool* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(bool)];
		memcpy(readBuf, sumBuf + offset, sizeof(bool));
		forRead = reinterpret_cast<bool*>(readBuf);
		m_HasAnimation.push_back(*forRead);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(bool);

		if (offset + sizeof(bool) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/


	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}

bool Model::LoadVertex(char* pFileName, unsigned int meshCount)
{
	std::vector<Vertex> vertexVector;
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Vertex_", meshCount, ".ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	Vertex* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(Vertex)];
		memcpy(readBuf, sumBuf + offset, sizeof(Vertex));
		forRead = reinterpret_cast<Vertex*>(readBuf);
		vertexVector.push_back(*forRead);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(Vertex);

		if (offset + sizeof(Vertex) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/

	m_Vertex.emplace(std::pair<unsigned int, std::vector<Vertex>>(meshCount, vertexVector));

	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}

bool Model::LoadVertexAnim(char* pFileName, unsigned int meshCount)
{
	std::vector<VertexAnim> vertexAnimVector;
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_VertexAnim_", meshCount, ".ksm");
	/***** 경로 작업 : 종료 *****/

	
	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	VertexAnim* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(VertexAnim)];
		memcpy(readBuf, sumBuf + offset, sizeof(VertexAnim));
		forRead = reinterpret_cast<VertexAnim*>(readBuf);
		vertexAnimVector.push_back(*forRead);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(VertexAnim);

		if (offset + sizeof(VertexAnim) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/

	m_VertexAnim.emplace(std::pair<unsigned int, std::vector<VertexAnim>>(meshCount, vertexAnimVector));

	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}

bool Model::LoadPolygon(char* pFileName, unsigned int meshCount)
{
	std::vector<polygon> polygonVector;
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Polygon_", meshCount, ".ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	polygon* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(polygon)];
		memcpy(readBuf, sumBuf + offset, sizeof(polygon));
		forRead = reinterpret_cast<polygon*>(readBuf);
		polygonVector.push_back(*forRead);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(polygon);

		if (offset + sizeof(polygon) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/

	m_polygon.push_back(polygonVector);

	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}

bool Model::LoadMaterial(char* pFileName, unsigned int meshCount)
{
	std::unordered_map<unsigned int, Material> materialVector;
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Material_", meshCount, ".ksm");
	/***** 경로 작업 : 종료 *****/

	
	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	Material* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(Material)];
		memcpy(readBuf, sumBuf + offset, sizeof(Material));
		forRead = reinterpret_cast<Material*>(readBuf);
		materialVector.emplace(std::pair<unsigned int, Material>(forRead->index, *forRead));

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(Material);

		if (offset + sizeof(Material) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/

	m_Material.emplace(std::pair<unsigned int, std::unordered_map<unsigned int, Material>>(meshCount, materialVector));

	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}

bool Model::LoadGlobalOffPosition(char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_GlobalOffPosition", ".ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	GlobalOffPosition* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	while (true)
	{
		readBuf = new char[sizeof(GlobalOffPosition)];
		memcpy(readBuf, sumBuf + offset, sizeof(GlobalOffPosition));
		forRead = reinterpret_cast<GlobalOffPosition*>(readBuf);
		m_GlobalOffPosition.push_back(forRead->globalOffPosition);

		XMFLOAT4X4 tempGlobalOffPosition;
		XMStoreFloat4x4(&tempGlobalOffPosition, forRead->globalOffPosition);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(GlobalOffPosition);

		if (offset + sizeof(GlobalOffPosition) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/


	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;


}

bool Model::LoadFinalTransform(char* pFileName, unsigned int meshCount, unsigned int animStackCount)
{
	std::list<char*> garbageCollector;

	/***** 경로 작업 : 시작 *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s%d%s", copy, "_FinalTransform_", meshCount, "_", animStackCount, ".ksm");
	/***** 경로 작업 : 종료 *****/


	/***** FBX 데이터 로드 : 시작 *****/
	FILE* pFileRB = _fsopen(result, "rb", SH_DENYNO);
	if (pFileRB == NULL)
	{
		return false;
	}

	// 버퍼에 저장된 임시데이터<사이즈, 데이터>를 저장하는 큐
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread가 읽은 개수
	char buf[BUFFER_SIZE]; // fread가 읽은 정보를 저장한 버퍼

					// 더이상 읽을게 없으면 0이하가 반환되므로 파일의 끝까지 읽음
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // 전부 큐에 넣어둠
	}

	int offset = 0; // 버퍼를 사이즈마다 자르면서 올려주는 버퍼 포인터의 시작위치
	char* readBuf = nullptr; //  데이터를 임시적으로 저장하는 버퍼
	ClusterEachFrame* forRead; // readBuf로 변환하여 저장하는 변수

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// 초기화 진행
	firstBuf = bufQueue.front().second;
	firstBufSize = bufQueue.front().first;
	bufQueue.pop();

	sumBuf = firstBuf;
	sumBufSize = firstBufSize;

	Animation anim;

	while (true)
	{
		readBuf = new char[sizeof(ClusterEachFrame)];
		memcpy(readBuf, sumBuf + offset, sizeof(ClusterEachFrame));
		forRead = reinterpret_cast<ClusterEachFrame*>(readBuf);

		// 존재하지 않는 키라면 새로 추가
		if (anim.m_Animation.find(forRead->index) == anim.m_Animation.end())
		{
			ClusterMatrix temp;
			temp.finalTransform.push_back(forRead->finalTransform);
			anim.m_Animation.emplace(std::pair<unsigned int, ClusterMatrix>(forRead->index, temp));
		}
		else // 존재하는 키라면
		{
			anim.m_Animation[forRead->index].finalTransform.push_back(forRead->finalTransform);
		}

		XMFLOAT4X4 tempFinalTransform;

		XMStoreFloat4x4(&tempFinalTransform, forRead->finalTransform);

		// 동적 할당한 readBuf 해제
		delete[] readBuf;

		offset += sizeof(ClusterEachFrame);

		if (offset + sizeof(ClusterEachFrame) > sumBufSize)
		{
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			secondBuf = bufQueue.front().second;
			secondBufSize = bufQueue.front().first;
			bufQueue.pop();

			sumBufSize = restBufSize + secondBufSize;

			sumBuf = new char[sumBufSize];
			garbageCollector.push_back(sumBuf);	// 나중에 한꺼번에 해재하기 위해 주소를 수집합니다.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // 첫번째 버퍼의 나머지 부분을 저장
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // 나머지 사이즈만큼 뒤에서 두번째 버퍼를 저장

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX 데이터 로드 : 종료 *****/


	/***** 로드한 데이터 저장 : 시작 *****/
	// Umap에 Animation 벡터가 없으면 새로 생성
	if (m_Animations.find(meshCount) == m_Animations.end())
	{
		std::vector<Animation> animVector;
		animVector.push_back(anim);
		m_Animations.emplace(std::pair<unsigned int, std::vector<Animation>>(meshCount, animVector));
	}
	else // 이미 있으면 추가
	{
		m_Animations.at(meshCount).push_back(anim);
	}
	/***** 로드한 데이터 저장 : 종료 *****/


	/***** 동적 할당 청소 : 시작 *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** 동적 할당 청소 : 종료 *****/

	return true;
}
/********** 파일 로딩 : 종료 **********/


/********** 지연 로딩 : 시작 **********/
Model::Model(const Model& rOther)
{
	m_hwnd = rOther.m_hwnd;

	m_ModelScaling = rOther.m_ModelScaling;
	m_ModelRotation = rOther.m_ModelRotation;
	m_ModelTranslation = rOther.m_ModelTranslation;

	m_DelayLoadingVertexBuffer = rOther.m_DelayLoadingVertexBuffer;
	m_DelayLoadingIndexBuffer = rOther.m_DelayLoadingIndexBuffer;

	m_DelayLoadingWorldMatrix = rOther.m_DelayLoadingWorldMatrix;

	m_DelayLoadingInitilized = rOther.m_DelayLoadingInitilized;
	m_DelayLoadingShader = rOther.m_DelayLoadingShader;

	m_DelayLoadingTexture = rOther.m_DelayLoadingTexture;

	m_CollisionType = rOther.m_CollisionType;
}

bool Model::DelayLoadingInitialize(ID3D11Device* pDevice, HWND hwnd, WCHAR* pTextureFileName, 
	XMFLOAT3 modelScaling, XMFLOAT3 modelRotation, XMFLOAT3 modelTranslation, int collisionType)
{
	m_hwnd = hwnd;

	m_ModelScaling = modelScaling;
	m_ModelRotation = modelRotation;
	m_ModelTranslation = modelTranslation;

	m_CollisionType = collisionType;

	if (!DelayLoadingBuffers(pDevice))
	{
		MessageBox(m_hwnd, L"Model.cpp : DelayLoadingBuffers(pDevice)", L"Error", MB_OK);
		return false;
	}

	if (!m_DelayLoadingTexture.Initialize(pDevice, m_hwnd, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_DelayLoadingTexture.Initialize()", L"Error", MB_OK);
		return false;
	}
	
	if (!m_DelayLoadingShader.Initialize(pDevice, m_hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_DelayLoadingShader.Initialize(pDevice, m_hwnd)", L"Error", MB_OK);
		return false;
	}

	/***** 지연 로딩 초기화 <뮤텍스> : 잠금 *****/
	m_DelayLoadingInitMutex.lock();
	m_DelayLoadingInitilized = true;
	m_DelayLoadingInitMutex.unlock();
	/***** 지연 로딩 초기화 <뮤텍스> : 해제 *****/

	return true;
}

bool Model::DelayLoadingBuffers(ID3D11Device* pDevice)
{
	HRESULT hResult;

	DelayLoadingVertexType* vertices = new DelayLoadingVertexType[4];
	vertices[0].position = XMFLOAT3(-1.0f, 3.0f, 0.0f);
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(1.0f, 3.0f, 0.0f);
	vertices[1].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[2].position = XMFLOAT3(1.0f, 0.0f, 0.0f);
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[3].position = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	vertices[3].texture = XMFLOAT2(0.0f, 1.0f);

	unsigned int* indices = new unsigned int[6];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 2;
	indices[4] = 3;
	indices[5] = 0;

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(DelayLoadingVertexType) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	hResult = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_DelayLoadingVertexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	hResult = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_DelayLoadingIndexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void Model::DelayLoadingRenderBuffers(ID3D11DeviceContext* pDeviceContext)
{
	// 정점 버퍼의 단위와 오프셋을 설정합니다.
	UINT stride = sizeof(DelayLoadingVertexType);
	UINT offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetVertexBuffers(0, 1, &m_DelayLoadingVertexBuffer, &stride, &offset);

	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetIndexBuffer(m_DelayLoadingIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::IsDelayLoadingInitilized()
{
	/***** 지연 로딩 초기화 <뮤텍스> : 잠금 *****/
	m_DelayLoadingInitMutex.lock();
	bool delayLoadingInitilized = m_DelayLoadingInitilized;
	m_DelayLoadingInitMutex.unlock();
	/***** 지연 로딩 초기화 <뮤텍스> : 해제 *****/

	return delayLoadingInitilized;
}
/********** 지연 로딩 : 종료 **********/


/********** 본 초기화 : 시작 **********/
Model::Model()
{
}

Model& Model::operator=(const Model& rOther)
{
	/* unordered_map 복사 2가지 방법
	m_VertexBufferUMap.insert(rOther.m_VertexBufferUMap.begin(), rOther.m_VertexBufferUMap.end());
	std::copy(rOther.m_VertexBufferUMap.begin(), rOther.m_VertexBufferUMap.end(), m_VertexBufferUMap);
	*/

	/* vector 복사 2가지 방법
	m_HasAnimation.assign(rOther.m_HasAnimation.begin(), rOther.m_HasAnimation.end());
	std::copy(rOther.m_HasAnimation.begin(), rOther.m_HasAnimation.end(), m_HasAnimation.begin());
	*/

	//m_ActiveFlag = rOther.m_ActiveFlag;

	m_ModelScaling = rOther.m_ModelScaling;

	m_VertexBufferUMap.insert(rOther.m_VertexBufferUMap.begin(), rOther.m_VertexBufferUMap.end());
	m_IndexBufferUMap.insert(rOther.m_IndexBufferUMap.begin(), rOther.m_IndexBufferUMap.end());
	m_AnimationVertexBufferUMap.insert(rOther.m_AnimationVertexBufferUMap.begin(), rOther.m_AnimationVertexBufferUMap.end());
	m_AnimationIndexBufferUMap.insert(rOther.m_AnimationIndexBufferUMap.begin(), rOther.m_AnimationIndexBufferUMap.end());

	//m_worldMatrix = rOther.m_worldMatrix;

	//m_DefaultLootAt = rOther.m_DefaultLootAt;
	//m_DefaultUp = rOther.m_DefaultUp;
	//m_DefaultSide = rOther.m_DefaultSide;

	//m_LootAt = rOther.m_LootAt;
	//m_Up = rOther.m_Up;
	//m_Side = rOther.m_Side;

	//m_cameraPosition = rOther.m_cameraPosition;

	//m_limitAngle = rOther.m_limitAngle;

	m_LocalLightShader = rOther.m_LocalLightShader;
	m_LocalLightAnimationShader = rOther.m_LocalLightAnimationShader;

	m_Texture = rOther.m_Texture;

	m_Info = rOther.m_Info;
	m_HasAnimation.assign(rOther.m_HasAnimation.begin(), rOther.m_HasAnimation.end());
	//m_Vertex.insert(rOther.m_Vertex.begin(), rOther.m_Vertex.end());
	//m_VertexAnim.insert(rOther.m_VertexAnim.begin(), rOther.m_VertexAnim.end());
	//m_polygon.assign(rOther.m_polygon.begin(), rOther.m_polygon.end());
	m_Material.insert(rOther.m_Material.begin(), rOther.m_Material.end());
	m_GlobalOffPosition.assign(rOther.m_GlobalOffPosition.begin(), rOther.m_GlobalOffPosition.end());
	m_Animations.insert(rOther.m_Animations.begin(), rOther.m_Animations.end());

	m_IndexSize.assign(rOther.m_IndexSize.begin(), rOther.m_IndexSize.end());

	m_AnimStackIndex = rOther.m_AnimStackIndex;
	m_AnimFrameCount = rOther.m_AnimFrameCount;
	m_SumDeltaTime = rOther.m_SumDeltaTime;

	for (int i = 0; i < BONE_FINAL_TRANSFORM_SIZE; i++)
	{
		m_FinalTransform[i] = rOther.m_FinalTransform[i];
	}

	m_SpecularZero = rOther.m_SpecularZero;
	for (int i = 0; i < MATERIAL_SIZE; i++)
	{
		m_AmbientColor[i] = rOther.m_AmbientColor[i];
		m_DiffuseColor[i] = rOther.m_DiffuseColor[i];
		m_SpecularPower[i] = rOther.m_SpecularPower[i];
		m_SpecularColor[i] = rOther.m_SpecularColor[i];
	}
	m_LightDirection = rOther.m_LightDirection;

	m_CollisionDetection.assign(rOther.m_CollisionDetection.begin(), rOther.m_CollisionDetection.end());

	m_Initilized = rOther.m_Initilized;

	return *this;
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* pDevice, char* pModelFileName, WCHAR* pTextureFileName, XMFLOAT3 modelScaling, bool specularZero, unsigned int animStackNum)
{
#ifdef _DEBUG
	printf("Start >> Model.cpp : Initialize()\n");
#endif

	// 머터리얼 값들 초기화
	m_SpecularZero = specularZero;

	m_AnimStackIndex = animStackNum;

	for (int i = 0; i < BONE_FINAL_TRANSFORM_SIZE; i++)
	{
		m_FinalTransform[i] = XMMatrixIdentity();
	}

	for (int i = 0; i < MATERIAL_SIZE; i++)
	{
		m_AmbientColor[i] = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		m_DiffuseColor[i] = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		m_SpecularPower[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		m_SpecularColor[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// 모델 데이터를 로드합니다.
	if (!LoadModel(pModelFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadModel(modelFilename)", L"Error", MB_OK);
		return false;
	}

	for (unsigned int mc = 0; mc < m_Info.meshCount; mc++)
	{
		// 애니메이션 유무 확인
		if (!m_HasAnimation.at(mc))
		{
			// 정점 및 인덱스 버퍼를 초기화합니다.
			if (!InitializeBuffers(pDevice, mc))
			{
				MessageBox(m_hwnd, L"Model.cpp : InitializeBuffers(device)", L"Error", MB_OK);
				return false;
			}

			// 인덱스 사이즈 저장
			m_IndexSize.push_back(m_Vertex.at(mc).size());
		}
		else
		{
			// 애니메이션 정점 및 인덱스 버퍼를 초기화합니다.
			if (!InitializeAnimationBuffers(pDevice, mc))
			{
				MessageBox(m_hwnd, L"Model.cpp : InitializeAnimationBuffers(device)", L"Error", MB_OK);
				return false;
			}

			// 인덱스 사이즈 저장
			m_IndexSize.push_back(m_VertexAnim.at(mc).size());
		}
	}

	// 이 모델의 텍스처를 로드합니다.
	if (!LoadTexture(pDevice, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadTexture(device, textureFilename)", L"Error", MB_OK);
		return false;
	}

	// LocalLightShader 객체 초기화
	if (!m_LocalLightShader.Initialize(pDevice, m_hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Initialize(device, m_hwnd)", L"Error", MB_OK);
		return false;
	}

	// LocalLightAnimationShader 객체 초기화
	if (!m_LocalLightAnimationShader.Initialize(pDevice, m_hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Initialize(device, m_hwnd)", L"Error", MB_OK);
		return false;
	}

	/***** 충돌 검사 초기화 : 시작 *****/
	for (unsigned int mc = 0; mc < m_Info.meshCount; mc++)
	{
		CollisionDetection collisionDetection;
		CalculateAABB(collisionDetection, mc);
		CalculateOBB(collisionDetection, mc);

		collisionDetection.Initilize(pDevice, m_hwnd, m_CollisionType);
		m_CollisionDetection.push_back(collisionDetection);
	}
	/***** 충돌 검사 초기화 : 종료 *****/

	/***** 본 초기화 <뮤텍스> : 잠금 *****/
	m_InitMutex.lock();
	m_ModelScaling = modelScaling; // 크기 재설정
	m_Initilized = true;
	m_InitMutex.unlock();
	/***** 본 초기화 <뮤텍스> : 해제 *****/

#ifdef _DEBUG
	printf("Success >> Model.cpp : Initialize()\n");
#endif

	return true;
}

void Model::Shutdown()
{
	for (auto iter = m_CollisionDetection.begin(); iter != m_CollisionDetection.end(); iter++)
	{
		iter->Shutdown();
	}
	m_CollisionDetection.clear();

	m_LocalLightAnimationShader.Shutdown();

	m_LocalLightShader.Shutdown();

	// 모델 텍스쳐를 반환합니다.
	ReleaseTexture();

	// 버텍스 및 인덱스 버퍼를 종료합니다.
	ShutdownBuffers();
}

bool Model::Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime, bool lineRenderFlag)
{
	// 초기화가 되었을 경우
	if (m_Initilized)
	{
		// 월드 매트릭스 계산
		m_worldMatrix = CalculateWorldMatrix();

		// Mesh 개수만큼 반복
		for (unsigned int mc = 0; mc < m_Info.meshCount; mc++)
		{
			// PixelShader에 넘겨줄 머터리얼 저장
			if (m_Info.hasMaterial) // 머터리얼을 가지고 있으며
			{
				for (auto iter = m_Material.at(mc).begin(); iter != m_Material.at(mc).end(); iter++)
				{
					m_AmbientColor[iter->first] = XMFLOAT4(iter->second.ambient.x, iter->second.ambient.y, iter->second.ambient.z, 1.0f);
					m_DiffuseColor[iter->first] = XMFLOAT4(iter->second.diffuse.x, iter->second.diffuse.y, iter->second.diffuse.z, 1.0f);
					m_SpecularPower[iter->first] = XMFLOAT4(static_cast<float>(iter->second.shininess), 0.0f, 0.0f, 0.0f);
					m_SpecularColor[iter->first] = XMFLOAT4(iter->second.specular.x, iter->second.specular.y, iter->second.specular.z, 1.0f);

					// 설정된 m_AmbientColor의 모든 값이 0.0f인 경우 보이지 않으므로 전부 0.05f로 변경
					if (m_AmbientColor[iter->first].x == 0.0f && m_AmbientColor[iter->first].y == 0.0f && m_AmbientColor[iter->first].z == 0.0f)
						m_AmbientColor[iter->first] = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);

					if (m_SpecularZero)
						m_SpecularColor[iter->first] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
				}
			}

			// 그리기를 준비하기 위해 그래픽 파이프 라인에 꼭지점과 인덱스 버퍼를 놓습니다.
			RenderBuffers(pDeviceContext, mc);

			XMMATRIX worldMatrix;
			worldMatrix = m_GlobalOffPosition.at(mc) * m_worldMatrix;

			// 애니메이션 유무에 따라 사용하는 쉐이더를 다르게 적용
			if (!m_HasAnimation.at(mc)) // 애니메이션이 없으면
			{
				// LocalLightShader 쉐이더를 사용하여 모델을 렌더링합니다.
				if (!m_LocalLightShader.Render(pDeviceContext, m_IndexSize.at(mc), worldMatrix, viewMatrix, projectionMatrix,
					m_Texture.GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor))
				{
					MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Render", L"Error", MB_OK);
					return false;
				}
			}
			else // 애니메이션이 있으면
			{
				m_AnimFrameSize = m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin()->second.finalTransform.size();

				for (auto iter = m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin(); iter != m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.end(); iter++)
				{
					m_FinalTransform[iter->first] = iter->second.finalTransform.at(m_AnimFrameCount);
				}

				// LocalLightAnimationShader 쉐이더를 사용하여 모델을 렌더링합니다.
				if (!m_LocalLightAnimationShader.Render(pDeviceContext, m_IndexSize.at(mc), worldMatrix, viewMatrix, projectionMatrix,
					m_Texture.GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor,
					m_FinalTransform, m_HasAnimation.at(mc)))
				{
					MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Render", L"Error", MB_OK);
					return false;
				}
			}

			m_CollisionDetection.at(mc).Render(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, m_CD_Side, m_CD_Up, m_CD_LootAt, m_ModelRotation, lineRenderFlag);
		}
	}
	else // 아직 초기화가 되지 않았을 경우 지연 로딩
	{
		pDirect3D->TurnOnAlphaBlending();

		/***** 지연 로딩 초기화 <뮤텍스> : 잠금 *****/
		m_DelayLoadingInitMutex.lock();
		if (m_DelayLoadingInitilized)
		{
			m_DelayLoadingInitMutex.unlock();
			/***** 지연 로딩 초기화 <뮤텍스> : 해제 *****/

			DelayLoadingRenderBuffers(pDeviceContext);

			// 아크 탄젠트 함수를 사용하여 현재 카메라 위치를 향하도록 빌보드 모델에 적용해야하는 회전을 계산합니다.
			float angle = static_cast<float>(atan2(m_ModelTranslation.x - cameraPosition.x, m_ModelTranslation.z - cameraPosition.z) * (180.0 / XM_PI));

			XMMATRIX sM = XMMatrixScaling(m_ModelScaling.x, m_ModelScaling.y, m_ModelScaling.z);
			XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(0.0f, angle * XM_RADIAN, 0.0f);
			XMMATRIX rM = XMMatrixRotationQuaternion(vQ);
			XMMATRIX tM = XMMatrixTranslation(m_ModelTranslation.x, m_ModelTranslation.y, m_ModelTranslation.z);
			m_DelayLoadingWorldMatrix = sM * rM * tM;

			m_DelayLoadingShader.Render(pDeviceContext, 6, m_DelayLoadingWorldMatrix, viewMatrix, projectionMatrix, m_DelayLoadingTexture.GetTexture());
			
			/***** 지연 로딩 초기화 <뮤텍스> : 잠금 *****/
			m_DelayLoadingInitMutex.lock();
		}
		m_DelayLoadingInitMutex.unlock();
		/***** 지연 로딩 초기화 <뮤텍스> : 해제 *****/

		pDirect3D->TurnOffAlphaBlending();
	}

	return true;
}

void Model::SetPosition(XMFLOAT3 position)
{
	m_ModelTranslation = position;
}
XMFLOAT3 Model::GetPosition()
{
	return m_ModelTranslation;
}
void Model::SetRotation(XMFLOAT3 rotation)
{
	m_ModelRotation = rotation;
}
XMFLOAT3 Model::GetRotation()
{
	return m_ModelRotation;
}
void Model::CalculateCameraPosition()
{
	// 1인칭
	//m_cameraPosition.x = m_ModelTranslation.x + (0.9f * m_LootAt.x);
	//m_cameraPosition.y = m_ModelTranslation.y + (2.0f * m_DefaultUp.y);
	//m_cameraPosition.z = m_ModelTranslation.z + (0.9f * m_LootAt.z);

	// 3인칭
	m_cameraPosition.x = m_ModelTranslation.x + (-10.0f * m_LootAt.x);
	m_cameraPosition.y = m_ModelTranslation.y + (4.0f * m_DefaultUp.y);
	m_cameraPosition.z = m_ModelTranslation.z + (-10.0f * m_LootAt.z);
}
XMFLOAT3 Model::GetCameraPosition()
{
	return m_cameraPosition;
}

void Model::SetActive(bool active)
{
	m_ActiveFlag = active;
}
bool Model::GetActive()
{
	return m_ActiveFlag;
}

bool Model::IsInitilized()
{
	/***** 본 초기화 <뮤텍스> : 잠금 *****/
	m_InitMutex.lock();
	bool initilized = m_Initilized;
	m_InitMutex.unlock();
	/***** 본 초기화 <뮤텍스> : 해제 *****/

	return initilized;
}

bool Model::LoadModel(char* pFileName)
{
	LoadFBX2KSM(pFileName);

	return true;
}

bool Model::InitializeBuffers(ID3D11Device* pDevice, unsigned int meshCount)
{
	HRESULT hResult;

	VertexType* vertices = new VertexType[m_Vertex.at(meshCount).size()];
	for (unsigned int i = 0; i < m_Vertex.at(meshCount).size(); i++)
	{
		vertices[i].position.x = m_Vertex.at(meshCount).at(i).position.x;
		vertices[i].position.y = m_Vertex.at(meshCount).at(i).position.y;
		vertices[i].position.z = m_Vertex.at(meshCount).at(i).position.z;

		vertices[i].texture.x = m_Vertex.at(meshCount).at(i).uv.x;
		vertices[i].texture.y = 1.0f - m_Vertex.at(meshCount).at(i).uv.y;

		vertices[i].normal.x = m_Vertex.at(meshCount).at(i).normal.x;
		vertices[i].normal.y = m_Vertex.at(meshCount).at(i).normal.y;
		vertices[i].normal.z = m_Vertex.at(meshCount).at(i).normal.z;
	}

	unsigned int* indices = new unsigned int[m_polygon.at(meshCount).size() * 3];
	for (unsigned int i = 0; i < m_polygon.at(meshCount).size(); i++)
	{
		indices[3 * i] = m_polygon.at(meshCount).at(i).indices[0];
		indices[3 * i + 1] = m_polygon.at(meshCount).at(i).indices[1];
		indices[3 * i + 2] = m_polygon.at(meshCount).at(i).indices[2];

		vertices[3 * i].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		vertices[3 * i + 1].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		vertices[3 * i + 2].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
	}

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_Vertex.at(meshCount).size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	ID3D11Buffer* vertexBuffer;
	
	hResult = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_VertexBufferUMap.emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, vertexBuffer));

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_polygon.at(meshCount).size() * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	ID3D11Buffer* indexBuffer;
	
	hResult = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_IndexBufferUMap.emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, indexBuffer));

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

bool Model::InitializeAnimationBuffers(ID3D11Device* pDevice, unsigned int meshCount)
{
	HRESULT hResult;

	AnimationVertexType* vertices = new AnimationVertexType[m_VertexAnim.at(meshCount).size()];
	for (unsigned int i = 0; i < m_VertexAnim.at(meshCount).size(); i++)
	{
		vertices[i].position.x = m_VertexAnim.at(meshCount).at(i).position.x;
		vertices[i].position.y = m_VertexAnim.at(meshCount).at(i).position.y;
		vertices[i].position.z = m_VertexAnim.at(meshCount).at(i).position.z;

		vertices[i].normal.x = m_VertexAnim.at(meshCount).at(i).normal.x;
		vertices[i].normal.y = m_VertexAnim.at(meshCount).at(i).normal.y;
		vertices[i].normal.z = m_VertexAnim.at(meshCount).at(i).normal.z;

		vertices[i].texture.x = m_VertexAnim.at(meshCount).at(i).uv.x;
		vertices[i].texture.y = 1.0f - m_VertexAnim.at(meshCount).at(i).uv.y;

		for (int j = 0; j < 4; j++)
		{
			vertices[i].blendingIndex[j] = m_VertexAnim.at(meshCount).at(i).boneIndex[j];
			vertices[i].blendingWeight[j] = m_VertexAnim.at(meshCount).at(i).boneWeight[j];
		}
	}

	unsigned int* indices = new unsigned int[m_polygon.at(meshCount).size() * 3];
	for (unsigned int i = 0; i < m_polygon.at(meshCount).size(); i++)
	{
		indices[3 * i] = m_polygon.at(meshCount).at(i).indices[0];
		indices[3 * i + 1] = m_polygon.at(meshCount).at(i).indices[1];
		indices[3 * i + 2] = m_polygon.at(meshCount).at(i).indices[2];

		vertices[3 * i].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		vertices[3 * i + 1].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		vertices[3 * i + 2].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
	}

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(AnimationVertexType) * m_VertexAnim.at(meshCount).size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	ID3D11Buffer* animationVertexBuffer;
	
	hResult = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &animationVertexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationVertexBufferUMap.emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationVertexBuffer));

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_polygon.at(meshCount).size() * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	ID3D11Buffer* animationIndexBuffer;
	hResult = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &animationIndexBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationIndexBufferUMap.emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationIndexBuffer));

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

bool Model::LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName)
{
	bool result;

	// 텍스처 오브젝트를 초기화한다.
	result = m_Texture.Initialize(pDevice, m_hwnd, pFileName);
	if (!result)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture.Initialize(device, filename)", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Model::RenderBuffers(ID3D11DeviceContext* pDeviceContext, unsigned int meshCount)
{
	if (!m_HasAnimation.at(meshCount))
	{
		// 정점 버퍼의 단위와 오프셋을 설정합니다.
		UINT stride = sizeof(VertexType);
		UINT offset = 0;

		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBufferUMap.at(meshCount), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_IndexBufferUMap.at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		// 정점 버퍼의 단위와 오프셋을 설정합니다.
		UINT stride = sizeof(AnimationVertexType);
		UINT offset = 0;

		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_AnimationVertexBufferUMap.at(meshCount), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_AnimationIndexBufferUMap.at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Model::ReleaseTexture()
{
	m_Texture.Shutdown();
	m_DelayLoadingTexture.Shutdown();
}

void Model::ShutdownBuffers()
{
	for (auto iter = m_AnimationIndexBufferUMap.begin(); iter != m_AnimationIndexBufferUMap.end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_AnimationIndexBufferUMap.clear();

	for (auto iter = m_AnimationVertexBufferUMap.begin(); iter != m_AnimationVertexBufferUMap.end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_AnimationVertexBufferUMap.clear();

	for (auto iter = m_IndexBufferUMap.begin(); iter != m_IndexBufferUMap.end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_IndexBufferUMap.clear();

	for (auto iter = m_VertexBufferUMap.begin(); iter != m_VertexBufferUMap.end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_VertexBufferUMap.clear();

	if (m_DelayLoadingIndexBuffer)
	{
		m_DelayLoadingIndexBuffer->Release();
		m_DelayLoadingIndexBuffer = nullptr;
	}

	if (m_DelayLoadingVertexBuffer)
	{
		m_DelayLoadingVertexBuffer->Release();
		m_DelayLoadingVertexBuffer = nullptr;
	}
}

bool Model::CheckFormat(char* pFileName) {
	if (Last4strcmp(pFileName, ".ksm") || Last4strcmp(pFileName, ".KSM"))
	{
		return true;
	}

	// 지원하는 포맷이 없으면 false 반환
	return false;
}
// 확장자 비교용 함수
bool Model::Last4strcmp(const char* pFileName, const char* pLast4FileName) {
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

XMMATRIX Model::CalculateWorldMatrix()
{
	XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(m_ModelRotation.x * XM_RADIAN, m_ModelRotation.y * XM_RADIAN, m_ModelRotation.z * XM_RADIAN);
	XMMATRIX rM = XMMatrixRotationQuaternion(vQ);

	XMStoreFloat3(&m_LootAt, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultLootAt), rM));
	XMStoreFloat3(&m_Up, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultUp), rM));
	XMStoreFloat3(&m_Side, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultSide), rM));

	XMMATRIX sM = XMMatrixScaling(m_ModelScaling.x, m_ModelScaling.y, m_ModelScaling.z);
	vQ = XMQuaternionRotationRollPitchYaw(0.0f, m_ModelRotation.y * XM_RADIAN, 0.0f);
	rM = XMMatrixRotationQuaternion(vQ);

	/***** 충돌 검사용 방향벡터들 계산 : 시작 *****/
	XMStoreFloat3(&m_CD_LootAt, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultLootAt), rM));
	XMStoreFloat3(&m_CD_Up, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultUp), rM));
	XMStoreFloat3(&m_CD_Side, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultSide), rM));
	/***** 충돌 검사용 방향벡터들 계산 : 종료 *****/

	XMMATRIX tM = XMMatrixTranslation(m_ModelTranslation.x, m_ModelTranslation.y, m_ModelTranslation.z);

	return sM * rM * tM;
}

void Model::CalculateAABB(CollisionDetection& rCollisionDetection, unsigned int meshCount)
{

	XMFLOAT3 min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 max = XMFLOAT3(0.0f, 0.0f, 0.0f);

	if (!m_HasAnimation.at(meshCount)) // 애니메이션이 없으면
	{
		for (auto iter = m_Vertex.at(meshCount).begin(); iter != m_Vertex.at(meshCount).end(); iter++)
		{
			if (iter->position.x <= min.x)
				min.x = iter->position.x;

			if (iter->position.y <= min.y)
				min.y = iter->position.y;

			if (iter->position.z <= min.z)
				min.z = iter->position.z;

			if (iter->position.x >= max.x)
				max.x = iter->position.x;

			if (iter->position.y >= max.y)
				max.y = iter->position.y;

			if (iter->position.z >= max.z)
				max.z = iter->position.z;
		}
	}
	else
	{
		for (auto iter = m_VertexAnim.at(meshCount).begin(); iter != m_VertexAnim.at(meshCount).end(); iter++)
		{
			if (iter->position.x <= min.x)
				min.x = iter->position.x;

			if (iter->position.y <= min.y)
				min.y = iter->position.y;

			if (iter->position.z <= min.z)
				min.z = iter->position.z;

			if (iter->position.x >= max.x)
				max.x = iter->position.x;

			if (iter->position.y >= max.y)
				max.y = iter->position.y;

			if (iter->position.z >= max.z)
				max.z = iter->position.z;
		}
	}

	rCollisionDetection.m_InitAABB.m_Min = min;
	rCollisionDetection.m_InitAABB.m_Max = max;
}

void Model::CalculateOBB(CollisionDetection& rCollisionDetection, unsigned int meshCount)
{
	XMFLOAT3 min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 max = XMFLOAT3(0.0f, 0.0f, 0.0f);

	if (!m_HasAnimation.at(meshCount)) // 애니메이션이 없으면
	{
		for (auto iter = m_Vertex.at(meshCount).begin(); iter != m_Vertex.at(meshCount).end(); iter++)
		{
			if (iter->position.x <= min.x)
				min.x = iter->position.x;

			if (iter->position.y <= min.y)
				min.y = iter->position.y;

			if (iter->position.z <= min.z)
				min.z = iter->position.z;

			if (iter->position.x >= max.x)
				max.x = iter->position.x;

			if (iter->position.y >= max.y)
				max.y = iter->position.y;

			if (iter->position.z >= max.z)
				max.z = iter->position.z;
		}
	}
	else
	{
		for (auto iter = m_VertexAnim.at(meshCount).begin(); iter != m_VertexAnim.at(meshCount).end(); iter++)
		{
			if (iter->position.x <= min.x)
				min.x = iter->position.x;

			if (iter->position.y <= min.y)
				min.y = iter->position.y;

			if (iter->position.z <= min.z)
				min.z = iter->position.z;

			if (iter->position.x >= max.x)
				max.x = iter->position.x;

			if (iter->position.y >= max.y)
				max.y = iter->position.y;

			if (iter->position.z >= max.z)
				max.z = iter->position.z;
		}
	}

	XMFLOAT3 center = XMFLOAT3((min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f, (min.z + max.z) / 2.0f);
	XMFLOAT3 extent = XMFLOAT3(max.x - center.x, max.y - center.y, max.z - center.z);

	rCollisionDetection.m_InitOBB.m_Center = center;
	rCollisionDetection.m_InitOBB.m_Extent = extent;
}
bool Model::Intersection(Model& rOther)
{
	bool intersection = false;

	for (int i = 0; i < m_CollisionDetection.size(); i++)
	{
		for (int j = 0; j < rOther.m_CollisionDetection.size(); j++)
		{
			// AABB AABB 교차 검사
			if (m_CollisionType == AxisAlignedBoundingBox && rOther.m_CollisionType == AxisAlignedBoundingBox)
			{
				if (m_CollisionDetection.at(i).IntersectionAABB(&m_CollisionDetection.at(i).m_AABB, &rOther.m_CollisionDetection.at(j).m_AABB))
				{
					m_CollisionDetection.at(i).SetCollisionCheck(true);
					rOther.m_CollisionDetection.at(j).SetCollisionCheck(true);
					intersection = true;
				}
			}
			// OBB OBB 교차 검사
			else if (m_CollisionType == OrientedBoundingBox && rOther.m_CollisionType == OrientedBoundingBox)
			{
				if (m_CollisionDetection.at(i).IntersectionOBB(&m_CollisionDetection.at(i).m_OBB, &rOther.m_CollisionDetection.at(j).m_OBB))
				{
					m_CollisionDetection.at(i).SetCollisionCheck(true);
					rOther.m_CollisionDetection.at(j).SetCollisionCheck(true);
					intersection = true;
				}
			}
			// AABB OBB 교차검사
			else // m_CollisionType != rOther.m_CollisionType
			{
				if (m_CollisionType == AxisAlignedBoundingBox)
				{
					if (m_CollisionDetection.at(i).IntersectionAABB_OBB(&m_CollisionDetection.at(i).m_AABB, &rOther.m_CollisionDetection.at(j).m_OBB))
					{
						m_CollisionDetection.at(i).SetCollisionCheck(true);
						rOther.m_CollisionDetection.at(j).SetCollisionCheck(true);
						intersection = true;
					}
				}
				else
				{
					if (m_CollisionDetection.at(i).IntersectionAABB_OBB(&rOther.m_CollisionDetection.at(j).m_AABB, &m_CollisionDetection.at(i).m_OBB))
					{
						m_CollisionDetection.at(i).SetCollisionCheck(true);
						rOther.m_CollisionDetection.at(j).SetCollisionCheck(true);
						intersection = true;
					}
				}
			}
		}
	}

	return intersection;
}

void Model::InitCollisionCheck()
{
	for (int i = 0; i < m_CollisionDetection.size(); i++)
	{
		m_CollisionDetection.at(i).SetCollisionCheck(false);
	}
}
/********** 본 초기화 : 종료 **********/

