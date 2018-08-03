#include "stdafx.h"
#include "Texture.h"
#include "LocalLightShader.h"
#include "LocalLightAnimationShader.h"
#include "HID.h"
#include "Model.h"

#include <fstream>

bool Model::LoadFBX2KSM(char* pFileName)
{
	// �ʱ� ���� ȹ��
	LoadFBXInfo(pFileName);
	LoadHasAnimation(pFileName);
	LoadGlobalOffPosition(pFileName);

	// �� �޽ø��� ����
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

		// ���͸����� �����ϸ�
		if (m_Info.hasMaterial)
		{
			LoadMaterial(pFileName, meshCount);
		}
	}

	// ����׿��� �� Ȯ�ο�
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
	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s", copy, "_info.ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
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
	/***** FBX ������ �ε� : ���� *****/

	return true;
}

bool Model::LoadHasAnimation(char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_HasAnimation_", ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	bool* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/


	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;
}

bool Model::LoadVertex(char* pFileName, unsigned int meshCount)
{
	std::vector<Vertex> vertexVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Vertex_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	Vertex* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/

	m_Vertex.emplace(std::pair<unsigned int, std::vector<Vertex>>(meshCount, vertexVector));

	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;
}

bool Model::LoadVertexAnim(char* pFileName, unsigned int meshCount)
{
	std::vector<VertexAnim> vertexAnimVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_VertexAnim_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/

	
	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	VertexAnim* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/

	m_VertexAnim.emplace(std::pair<unsigned int, std::vector<VertexAnim>>(meshCount, vertexAnimVector));

	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;
}

bool Model::LoadPolygon(char* pFileName, unsigned int meshCount)
{
	std::vector<polygon> polygonVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Polygon_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	polygon* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/

	m_polygon.push_back(polygonVector);

	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;
}

bool Model::LoadMaterial(char* pFileName, unsigned int meshCount)
{
	std::unordered_map<unsigned int, Material> materialVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Material_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/

	
	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	Material* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/

	m_Material.emplace(std::pair<unsigned int, std::unordered_map<unsigned int, Material>>(meshCount, materialVector));

	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;
}

bool Model::LoadGlobalOffPosition(char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_GlobalOffPosition", ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	GlobalOffPosition* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/


	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

	return true;


}

bool Model::LoadFinalTransform(char* pFileName, unsigned int meshCount, unsigned int animStackCount)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s%d%s", copy, "_FinalTransform_", meshCount, "_", animStackCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[BUFFER_SIZE]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		char* temp = new char[bufLen];
		garbageCollector.push_back(temp); // ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
		memcpy(temp, buf, bufLen);
		bufQueue.push(std::pair<unsigned int, char*>(bufLen, temp)); // ���� ť�� �־��
	}

	int offset = 0; // ���۸� ������� �ڸ��鼭 �÷��ִ� ���� �������� ������ġ
	char* readBuf = nullptr; //  �����͸� �ӽ������� �����ϴ� ����
	ClusterEachFrame* forRead; // readBuf�� ��ȯ�Ͽ� �����ϴ� ����

	char* firstBuf = nullptr;
	unsigned firstBufSize = 0;

	char* secondBuf = nullptr;
	unsigned secondBufSize = 0;

	char* sumBuf = nullptr;
	unsigned sumBufSize = 0;

	// �ʱ�ȭ ����
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

		// �������� �ʴ� Ű��� ���� �߰�
		if (anim.m_Animation.find(forRead->index) == anim.m_Animation.end())
		{
			ClusterMatrix temp;
			temp.finalTransform.push_back(forRead->finalTransform);
			anim.m_Animation.emplace(std::pair<unsigned int, ClusterMatrix>(forRead->index, temp));
		}
		else // �����ϴ� Ű���
		{
			anim.m_Animation[forRead->index].finalTransform.push_back(forRead->finalTransform);
		}

		XMFLOAT4X4 tempFinalTransform;

		XMStoreFloat4x4(&tempFinalTransform, forRead->finalTransform);

		// ���� �Ҵ��� readBuf ����
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
			garbageCollector.push_back(sumBuf);	// ���߿� �Ѳ����� �����ϱ� ���� �ּҸ� �����մϴ�.
			if (restBufSize > 0)
				memcpy(sumBuf, firstBuf + offset, restBufSize); // ù��° ������ ������ �κ��� ����
			memcpy(sumBuf + restBufSize, secondBuf, secondBufSize); // ������ �����ŭ �ڿ��� �ι�° ���۸� ����

			firstBuf = sumBuf;
			offset = 0;
		}
	}

	fclose(pFileRB);
	/***** FBX ������ �ε� : ���� *****/


	/***** �ε��� ������ ���� : ���� *****/
	// Umap�� Animation ���Ͱ� ������ ���� ����
	if (m_Animations.find(meshCount) == m_Animations.end())
	{
		std::vector<Animation> animVector;
		animVector.push_back(anim);
		m_Animations.emplace(std::pair<unsigned int, std::vector<Animation>>(meshCount, animVector));
	}
	else // �̹� ������ �߰�
	{
		m_Animations.at(meshCount).push_back(anim);
	}
	/***** �ε��� ������ ���� : ���� *****/


	/***** ���� �Ҵ� û�� : ���� *****/
	for (auto iter = garbageCollector.begin(); iter != garbageCollector.end(); iter++)
		delete (*iter);
	garbageCollector.clear();
	/***** ���� �Ҵ� û�� : ���� *****/

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

	// �ʱ� ���� ��Ʈ���� ����
	m_ModelScaling = modelcaling;
	m_ModelRotation = modelRotation;
	m_ModelTranslation = modelTranslation;

	// ���͸��� ���� �ʱ�ȭ
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

	// �� �����͸� �ε��մϴ�.
	if (!LoadModel(pModelFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadModel(modelFilename)", L"Error", MB_OK);
		return false;
	}

	for (unsigned int mc = 0; mc < m_Info.meshCount; mc++)
	{
		// �ִϸ��̼� ���� Ȯ��
		if (!m_HasAnimation.at(mc))
		{
			// ���� �� �ε��� ���۸� �ʱ�ȭ�մϴ�.
			if (!InitializeBuffers(pDevice, mc))
			{
				MessageBox(m_hwnd, L"Model.cpp : InitializeBuffers(device)", L"Error", MB_OK);
				return false;
			}
		}
		else
		{
			// �ִϸ��̼� ���� �� �ε��� ���۸� �ʱ�ȭ�մϴ�.
			if (!InitializeAnimationBuffers(pDevice, mc))
			{
				MessageBox(m_hwnd, L"Model.cpp : InitializeAnimationBuffers(device)", L"Error", MB_OK);
				return false;
			}
		}
	}

	// �� ���� �ؽ�ó�� �ε��մϴ�.
	if (!LoadTexture(pDevice, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadTexture(device, textureFilename)", L"Error", MB_OK);
		return false;
	}

	// LocalLightShader ��ü ����
	m_LocalLightShader = new LocalLightShader;
	if (!m_LocalLightShader)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader = new LocalLightShader;", L"Error", MB_OK);
		return false;
	}

	// LocalLightShader ��ü �ʱ�ȭ
	if (!m_LocalLightShader->Initialize(pDevice, hwnd))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Initialize(device, hwnd)", L"Error", MB_OK);
		return false;
	}

	// LocalLightAnimationShader ��ü ����
	m_LocalLightAnimationShader = new LocalLightAnimationShader;
	if (!m_LocalLightAnimationShader)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader = new m_LocalLightAnimationShader;", L"Error", MB_OK);
		return false;
	}

	// LocalLightAnimationShader ��ü �ʱ�ȭ
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
	// LocalLightAnimationShader ��ü ��ȯ
	if (m_LocalLightAnimationShader)
	{
		m_LocalLightAnimationShader->Shutdown();
		delete m_LocalLightAnimationShader;
		m_LocalLightAnimationShader = nullptr;
	}

	// LocalLightShader ��ü ��ȯ
	if (m_LocalLightShader)
	{
		m_LocalLightShader->Shutdown();
		delete m_LocalLightShader;
		m_LocalLightShader = nullptr;
	}
	
	// �� �ؽ��ĸ� ��ȯ�մϴ�.
	ReleaseTexture();

	// ���ؽ� �� �ε��� ���۸� �����մϴ�.
	ShutdownBuffers();
}

bool Model::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime)
{
	// ���� ��Ʈ���� ���
	m_worldMatrix = CalculateWorldMatrix();

	// Mesh ������ŭ �ݺ�
	for (unsigned int mc = 0; mc < m_Info.meshCount; mc++)
	{
		// PixelShader�� �Ѱ��� ���͸��� ����
		if (m_Info.hasMaterial) // ���͸����� ������ ������
		{
			for (auto iter = m_Material.at(mc).begin(); iter != m_Material.at(mc).end(); iter++)
			{
				m_AmbientColor[iter->first] = XMFLOAT4(iter->second.ambient.x, iter->second.ambient.y, iter->second.ambient.z, 1.0f);
				m_DiffuseColor[iter->first] = XMFLOAT4(iter->second.diffuse.x, iter->second.diffuse.y, iter->second.diffuse.z, 1.0f);
				m_SpecularPower[iter->first] = XMFLOAT4(static_cast<float>(iter->second.shininess), 0.0f, 0.0f, 0.0f);
				m_SpecularColor[iter->first] = XMFLOAT4(iter->second.specular.x, iter->second.specular.y, iter->second.specular.z, 1.0f);

				// ������ m_AmbientColor�� ��� ���� 0.0f�� ��� ������ �����Ƿ� ���� 0.05f�� ����
				if (m_AmbientColor[iter->first].x == 0.0f && m_AmbientColor[iter->first].y == 0.0f && m_AmbientColor[iter->first].z == 0.0f)
					m_AmbientColor[iter->first] = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);

				if (m_SpecularZero)
					m_SpecularColor[iter->first] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			}
		}

		// �׸��⸦ �غ��ϱ� ���� �׷��� ������ ���ο� �������� �ε��� ���۸� �����ϴ�.
		RenderBuffers(pDeviceContext, mc);

		XMMATRIX worldMatrix;
		worldMatrix = m_GlobalOffPosition.at(mc) * m_worldMatrix;

		// �ִϸ��̼� ������ ���� ����ϴ� ���̴��� �ٸ��� ����
		if (!m_HasAnimation.at(mc)) // �ִϸ��̼��� ������
		{
			// LocalLightShader ���̴��� ����Ͽ� ���� �������մϴ�.
			if (!m_LocalLightShader->Render(pDeviceContext, m_Vertex.at(mc).size(), worldMatrix, viewMatrix, projectionMatrix,
				m_Texture->GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor))
			{
				MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Render", L"Error", MB_OK);
				return false;
			}
		}
		else // �ִϸ��̼��� ������
		{
			/***** �ִϸ��̼� ��� ���� : ���� *****/
			m_SumDeltaTime += deltaTime;
			if (m_SumDeltaTime > 41.66f) // 1�ʴ� 24������
			{
				m_AnimFrameCount++;
				if (m_AnimFrameCount >= m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin()->second.finalTransform.size())
				{
					m_AnimFrameCount = 0;
				}
				m_SumDeltaTime = 0.0f;
			}
			/***** �ִϸ��̼� ��� ���� : ���� *****/

			for (auto iter = m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.begin(); iter != m_Animations.at(mc).at(m_AnimStackIndex).m_Animation.end(); iter++)
			{
				m_FinalTransform[iter->first] = iter->second.finalTransform.at(m_AnimFrameCount);
			}

			// LocalLightAnimationShader ���̴��� ����Ͽ� ���� �������մϴ�.
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


	// ���� ����
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
	// ������ ���� 1��Ī���� 3��Ī���� ����˴ϴ�.
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

	// ���� ���� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_Vertex.at(meshCount).size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ���۸� ����ϴ�.
	ID3D11Buffer* vertexBuffer;
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_VertexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, vertexBuffer));

	// ���� �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_polygon.at(meshCount).size() * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ��մϴ�.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� �����մϴ�.
	ID3D11Buffer* indexBuffer;
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_IndexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, indexBuffer));

	// �����ǰ� ���� �Ҵ�� ���� ���ۿ� �ε��� ���۸� �����մϴ�.
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

	// ���� ���� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(AnimationVertexType) * m_VertexAnim.at(meshCount).size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ���۸� ����ϴ�.
	ID3D11Buffer* animationVertexBuffer;
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &animationVertexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationVertexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationVertexBuffer));

	// ���� �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_polygon.at(meshCount).size() * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ��մϴ�.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� �����մϴ�.
	ID3D11Buffer* animationIndexBuffer;
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &animationIndexBuffer)))
	{
		MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
		return false;
	}
	m_AnimationIndexBuffer->emplace(std::pair<unsigned int, ID3D11Buffer*>(meshCount, animationIndexBuffer));

	// �����ǰ� ���� �Ҵ�� ���� ���ۿ� �ε��� ���۸� �����մϴ�.
	delete[] m_vertices;
	m_vertices = nullptr;

	delete[] m_indices;
	m_indices = nullptr;

	return true;
}

bool Model::LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName)
{
	// �ؽ�ó ������Ʈ�� �����Ѵ�.
	m_Texture = new Texture;
	if (!m_Texture)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture = new Texture;", L"Error", MB_OK);
		return false;
	}

	// �ؽ�ó ������Ʈ�� �ʱ�ȭ�Ѵ�.
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
		// ���� ������ ������ �������� �����մϴ�.
		UINT stride = sizeof(VertexType);
		UINT offset = 0;

		// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer->at(meshCount), &stride, &offset);

		// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
		pDeviceContext->IASetIndexBuffer(m_IndexBuffer->at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		// ���� ������ ������ �������� �����մϴ�.
		UINT stride = sizeof(AnimationVertexType);
		UINT offset = 0;

		// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_AnimationVertexBuffer->at(meshCount), &stride, &offset);

		// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
		pDeviceContext->IASetIndexBuffer(m_AnimationIndexBuffer->at(meshCount), DXGI_FORMAT_R32_UINT, 0);

		// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Model::ReleaseTexture()
{
	// �ؽ�ó ������Ʈ�� �������Ѵ�.
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

	// �����ϴ� ������ ������ false ��ȯ
	return false;
}
// Ȯ���� �񱳿� �Լ�
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