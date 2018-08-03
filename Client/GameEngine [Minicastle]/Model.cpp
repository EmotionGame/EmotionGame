#include "stdafx.h"
#include "Texture.h"
#include "LocalLightShader.h"
#include "LocalLightAnimationShader.h"
#include "HID.h"
#include "Model.h"

#include <fstream>

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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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
	FILE* pFileRB = fopen(result, "rb");
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

Model::Model()
{
}

Model::Model(const Model& other)
{
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, char* pModelFileName, WCHAR* pTextureFileName,
	XMFLOAT3 modelcaling, XMFLOAT3 modelRotation, XMFLOAT3 modelTranslation, bool specularZero, unsigned int animStackNum)
{
#ifdef _DEBUG
	printf("Start >> Model.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;
	m_HID = pHID;

	// 초기 월드 매트릭스 계산용
	m_ModelScaling = modelcaling;
	m_ModelRotation = modelRotation;
	m_ModelTranslation = modelTranslation;

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
		}
		else
		{
			// 애니메이션 정점 및 인덱스 버퍼를 초기화합니다.
			if (!InitializeAnimationBuffers(pDevice, mc))
			{
				MessageBox(m_hwnd, L"Model.cpp : InitializeAnimationBuffers(device)", L"Error", MB_OK);
				return false;
			}
		}
	}

	// 이 모델의 텍스처를 로드합니다.
	if (!LoadTexture(pDevice, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadTexture(device, textureFilename)", L"Error", MB_OK);
		return false;
	}

	// LocalLightShader 객체 생성
	m_LocalLightShader = new LocalLightShader;
	if (!m_LocalLightShader)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader = new LocalLightShader;", L"Error", MB_OK);
		return false;
	}

	// LocalLightShader 객체 초기화
	if (!m_LocalLightShader->Initialize(pDevice, hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Initialize(device, hwnd)", L"Error", MB_OK);
		return false;
	}

	// LocalLightAnimationShader 객체 생성
	m_LocalLightAnimationShader = new LocalLightAnimationShader;
	if (!m_LocalLightAnimationShader)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader = new m_LocalLightAnimationShader;", L"Error", MB_OK);
		return false;
	}

	// LocalLightAnimationShader 객체 초기화
	if (!m_LocalLightAnimationShader->Initialize(pDevice, hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Initialize(device, hwnd)", L"Error", MB_OK);
		return false;
	}

	m_InitMutex.lock();
	m_Initilized = true;
	m_InitMutex.unlock();

#ifdef _DEBUG
	printf("Success >> Model.cpp : Initialize()\n");
#endif

	return true;
}

void Model::Shutdown()
{
	// LocalLightAnimationShader 객체 반환
	if (m_LocalLightAnimationShader)
	{
		m_LocalLightAnimationShader->Shutdown();
		delete m_LocalLightAnimationShader;
		m_LocalLightAnimationShader = nullptr;
	}

	// LocalLightShader 객체 반환
	if (m_LocalLightShader)
	{
		m_LocalLightShader->Shutdown();
		delete m_LocalLightShader;
		m_LocalLightShader = nullptr;
	}
	
	// 모델 텍스쳐를 반환합니다.
	ReleaseTexture();

	// 버텍스 및 인덱스 버퍼를 종료합니다.
	ShutdownBuffers();
}

bool Model::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
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
			if (!m_LocalLightShader->Render(pDeviceContext, m_Vertex.at(mc).size(), worldMatrix, viewMatrix, projectionMatrix,
				m_Texture->GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor))
			{
				MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Render", L"Error", MB_OK);
				return false;
			}
		}
		else // 애니메이션이 있으면
		{
			/***** 애니메이션 재생 관리 : 시작 *****/
			m_SumDeltaTime += deltaTime;
			if (m_SumDeltaTime > 41.66f) // 1초당 24프레임
			{
				m_AnimFrameCount++;
				if (m_AnimFrameCount >= m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin()->second.finalTransform.size())
				{
					m_AnimFrameCount = 0;
				}
				m_SumDeltaTime = 0.0f;
			}
			/***** 애니메이션 재생 관리 : 종료 *****/

			for (auto iter = m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin(); iter != m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.end(); iter++)
			{
				m_FinalTransform[iter->first] = iter->second.finalTransform.at(m_AnimFrameCount);
			}

			// LocalLightAnimationShader 쉐이더를 사용하여 모델을 렌더링합니다.
			if (!m_LocalLightAnimationShader->Render(pDeviceContext, m_VertexAnim.at(mc).size(), worldMatrix, viewMatrix, projectionMatrix,
				m_Texture->GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor,
				m_FinalTransform, m_HasAnimation.at(mc)))
			{
				MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Render", L"Error", MB_OK);
				return false;
			}
		}
	}

	return true;
}

void Model::MoveObejctToLookAt(XMFLOAT3 value)
{
	m_ModelTranslation.x += value.x * m_LootAt.x;
	//m_ModelTranslation.y += value.y * m_LootAt.y;
	m_ModelTranslation.z += value.z * m_LootAt.z;
}
void Model::MoveObjectToLookAtUp(XMFLOAT3 value)
{
	//m_ModelTranslation.x += value.x * m_Up.x;
	m_ModelTranslation.y += value.y * m_Up.y;
	//m_ModelTranslation.z += value.z * m_Up.z;
}
void Model::MoveObejctToLookAtSide(XMFLOAT3 value)
{
	m_ModelTranslation.x += value.x * m_Side.x;
	//m_ModelTranslation.y += value.y * m_Side.y;
	m_ModelTranslation.z += value.z * m_Side.z;
}
void Model::RotateObject(XMFLOAT3 value)
{
	if (m_ModelRotation.x >= 360.0f)
		m_ModelRotation.x += -360.0f;
	if (m_ModelRotation.x <= -360.0f)
		m_ModelRotation.x += 360.0f;

	if (m_ModelRotation.y >= 360.0f)
		m_ModelRotation.y += -360.0f;
	if (m_ModelRotation.y <= -360.0f)
		m_ModelRotation.y += 360.0f;

	if (m_ModelRotation.z >= 360.0f)
		m_ModelRotation.z += -360.0f;
	if (m_ModelRotation.z <= -360.0f)
		m_ModelRotation.z += 360.0f;

	m_ModelRotation.y += value.y;
	m_ModelRotation.z += value.z;


	// 각도 제한
	float result = m_ModelRotation.x += value.x;

	if (-m_limitAngle <= result && result <= m_limitAngle)
		m_ModelRotation.x += value.x;
	else if (result < -m_limitAngle)
		m_ModelRotation.x = -m_limitAngle;
	else if (m_limitAngle < result)
		m_ModelRotation.x = m_limitAngle;
}
void Model::PlayerControl(float frameTime)
{
	int mouseX, mouseY;
	m_HID->GetMouse_Keyboard()->GetDeltaMouse(mouseX, mouseY);

	float moveSpeed = 0.05f * frameTime;
	float rotateSpeed = 0.05f * frameTime;

	RotateObject(XMFLOAT3(rotateSpeed * mouseY, rotateSpeed * mouseX, 0.0f));

	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_W))
		MoveObejctToLookAt(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_S))
		MoveObejctToLookAt(XMFLOAT3(-moveSpeed, -moveSpeed, -moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_A))
		MoveObejctToLookAtSide(XMFLOAT3(-moveSpeed, -moveSpeed, -moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_D))
		MoveObejctToLookAtSide(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_E))
		MoveObjectToLookAtUp(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_Q))
		MoveObjectToLookAtUp(XMFLOAT3 (-moveSpeed, -moveSpeed, -moveSpeed));

	CalculateCameraPosition();
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
	// 조절에 따라 1인칭에서 3인칭으로 변경됩니다.
	m_cameraPosition.x = m_ModelTranslation.x + (-20.0f * m_LootAt.x);
	m_cameraPosition.y = m_ModelTranslation.y + (6.5f * m_DefaultUp.y);
	m_cameraPosition.z = m_ModelTranslation.z + (-20.0f * m_LootAt.z);
}
XMFLOAT3 Model::GetCameraPosition()
{
	return m_cameraPosition;
}

bool Model::IsActive()
{
	return m_ActiveFlag;
}

bool Model::IsInitilized()
{
	m_InitMutex.lock();
	bool init = m_Initilized;
	m_InitMutex.unlock();

	return init;
}

void Model::SetInitStarted(bool initStart)
{
	m_InitStartMutex.lock();
	m_InitSatrt = initStart;
	m_InitStartMutex.unlock();
}
bool Model::GetInitStarted()
{
	m_InitStartMutex.lock();
	bool initStart = m_InitSatrt;
	m_InitStartMutex.unlock();

	return initStart;
}

bool Model::LoadModel(char* pFileName)
{
	LoadFBX2KSM(pFileName);

	return true;
}

bool Model::InitializeBuffers(ID3D11Device* pDevice, unsigned int meshCount)
{
	VertexType* m_vertices = new VertexType[m_Vertex.at(meshCount).size()];
	for (unsigned int i = 0; i < m_Vertex.at(meshCount).size(); i++)
	{
		m_vertices[i].position.x = m_Vertex.at(meshCount).at(i).position.x;
		m_vertices[i].position.y = m_Vertex.at(meshCount).at(i).position.y;
		m_vertices[i].position.z = m_Vertex.at(meshCount).at(i).position.z;

		m_vertices[i].texture.x = m_Vertex.at(meshCount).at(i).uv.x;
		m_vertices[i].texture.y = 1.0f - m_Vertex.at(meshCount).at(i).uv.y;

		m_vertices[i].normal.x = m_Vertex.at(meshCount).at(i).normal.x;
		m_vertices[i].normal.y = m_Vertex.at(meshCount).at(i).normal.y;
		m_vertices[i].normal.z = m_Vertex.at(meshCount).at(i).normal.z;
	}

	unsigned int* m_indices = new unsigned int[m_polygon.at(meshCount).size() * 3];
	for (unsigned int i = 0; i < m_polygon.at(meshCount).size(); i++)
	{
		m_indices[3 * i] = m_polygon.at(meshCount).at(i).indices[0];
		m_indices[3 * i + 1] = m_polygon.at(meshCount).at(i).indices[1];
		m_indices[3 * i + 2] = m_polygon.at(meshCount).at(i).indices[2];

		m_vertices[3 * i].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		m_vertices[3 * i + 1].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		m_vertices[3 * i + 2].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
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
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	ID3D11Buffer* vertexBuffer;
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_VertexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, vertexBuffer));

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
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	ID3D11Buffer* indexBuffer;
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_IndexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, indexBuffer));

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
	delete[] m_vertices;
	m_vertices = nullptr;

	delete[] m_indices;
	m_indices = nullptr;

	return true;
}

bool Model::InitializeAnimationBuffers(ID3D11Device* pDevice, unsigned int meshCount)
{
	AnimationVertexType* m_vertices = new AnimationVertexType[m_VertexAnim.at(meshCount).size()];
	for (unsigned int i = 0; i < m_VertexAnim.at(meshCount).size(); i++)
	{
		m_vertices[i].position.x = m_VertexAnim.at(meshCount).at(i).position.x;
		m_vertices[i].position.y = m_VertexAnim.at(meshCount).at(i).position.y;
		m_vertices[i].position.z = m_VertexAnim.at(meshCount).at(i).position.z;

		m_vertices[i].normal.x = m_VertexAnim.at(meshCount).at(i).normal.x;
		m_vertices[i].normal.y = m_VertexAnim.at(meshCount).at(i).normal.y;
		m_vertices[i].normal.z = m_VertexAnim.at(meshCount).at(i).normal.z;

		m_vertices[i].texture.x = m_VertexAnim.at(meshCount).at(i).uv.x;
		m_vertices[i].texture.y = 1.0f - m_VertexAnim.at(meshCount).at(i).uv.y;

		for (int j = 0; j < 4; j++)
		{
			m_vertices[i].blendingIndex[j] = m_VertexAnim.at(meshCount).at(i).boneIndex[j];
			m_vertices[i].blendingWeight[j] = m_VertexAnim.at(meshCount).at(i).boneWeight[j];
		}
	}

	unsigned int* m_indices = new unsigned int[m_polygon.at(meshCount).size() * 3];
	for (unsigned int i = 0; i < m_polygon.at(meshCount).size(); i++)
	{
		m_indices[3 * i] = m_polygon.at(meshCount).at(i).indices[0];
		m_indices[3 * i + 1] = m_polygon.at(meshCount).at(i).indices[1];
		m_indices[3 * i + 2] = m_polygon.at(meshCount).at(i).indices[2];

		m_vertices[3 * i].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		m_vertices[3 * i + 1].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
		m_vertices[3 * i + 2].materialIndex = m_polygon.at(meshCount).at(i).materialIndex;
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
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	ID3D11Buffer* animationVertexBuffer;
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &animationVertexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationVertexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationVertexBuffer));

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
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	ID3D11Buffer* animationIndexBuffer;
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &animationIndexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationIndexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationIndexBuffer));

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
	delete[] m_vertices;
	m_vertices = nullptr;

	delete[] m_indices;
	m_indices = nullptr;

	return true;
}

bool Model::LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName)
{
	// 텍스처 오브젝트를 생성한다.
	m_Texture = new Texture;
	if (!m_Texture)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture = new Texture;", L"Error", MB_OK);
		return false;
	}

	// 텍스처 오브젝트를 초기화한다.
	if (!m_Texture->Initialize(pDevice, m_hwnd, pFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture->Initialize(device, filename)", L"Error", MB_OK);
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
		pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer->at(meshCount), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_IndexBuffer->at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		// 정점 버퍼의 단위와 오프셋을 설정합니다.
		UINT stride = sizeof(AnimationVertexType);
		UINT offset = 0;

		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_AnimationVertexBuffer->at(meshCount), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_AnimationIndexBuffer->at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Model::ReleaseTexture()
{
	// 텍스처 오브젝트를 릴리즈한다.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}

void Model::ShutdownBuffers()
{
	for (auto iter = m_AnimationIndexBuffer->begin(); iter != m_AnimationIndexBuffer->end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_AnimationIndexBuffer->clear();
	delete m_AnimationIndexBuffer;
	m_AnimationIndexBuffer = nullptr;

	for (auto iter = m_AnimationVertexBuffer->begin(); iter != m_AnimationVertexBuffer->end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_AnimationVertexBuffer->clear();
	delete m_AnimationVertexBuffer;
	m_AnimationVertexBuffer = nullptr;

	for (auto iter = m_IndexBuffer->begin(); iter != m_IndexBuffer->end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_IndexBuffer->clear();
	delete m_IndexBuffer;
	m_IndexBuffer = nullptr;

	for (auto iter = m_VertexBuffer->begin(); iter != m_VertexBuffer->end(); iter++)
	{
		iter->second->Release();
		iter->second = nullptr;
	}
	m_VertexBuffer->clear();
	delete m_VertexBuffer;
	m_VertexBuffer = nullptr;
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
	XMMATRIX tM = XMMatrixTranslation(m_ModelTranslation.x, m_ModelTranslation.y, m_ModelTranslation.z);

	return sM * rM * tM;
}