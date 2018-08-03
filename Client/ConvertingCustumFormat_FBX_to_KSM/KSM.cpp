#include "stdafx.h"
#include "FBX.h"
#include "KSM.h"

bool KSM::SaveFBX2KSM(FBX* pFBX, char* pFileName)
{
	char deletedFormatStr[MAX_PATH];

	// .fbx �ڸ���
	strncpy_s(deletedFormatStr, strlen(pFileName) - 3, pFileName, strlen(pFileName) - 4);

	// �ʱ� ���� ȹ��
	SaveFBXInfo(pFBX, deletedFormatStr);
	SaveHasAnimation(pFBX, deletedFormatStr);
	SaveGlobalOffPosition(pFBX, deletedFormatStr);

	// �� �޽ø��� ����
	for (unsigned int meshCount = 0; meshCount < pFBX->m_MeshCount; meshCount++)
	{
		if (pFBX->m_HasAnimation->at(meshCount))
		{
			SaveVertexAnim(pFBX, deletedFormatStr, meshCount);

			for (unsigned int animStackCount = 0; animStackCount < pFBX->m_AnimationStackVector->size(); animStackCount++)
			{
				SaveFinalTransform(pFBX, deletedFormatStr, meshCount, animStackCount);
			}
		}
		else
		{
			SaveVertex(pFBX, deletedFormatStr, meshCount);
		}

		SavePolygon(pFBX, deletedFormatStr, meshCount);

		// ���͸����� �����ϸ�
		if (pFBX->m_MaterialLookUpVector->size() > 0)
		{
			SaveMaterial(pFBX, deletedFormatStr, meshCount);
		}
	}

	// ����׿��� �� Ȯ�ο�
	pFBX;
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

bool KSM::SaveFBXInfo(FBX* pFBX, char* pFileName)
{
	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s", copy, "_info.ksm");
	printf("result : %s\n", result);
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp SaveFBXInfo(FBX* pFBX, char* pFileName) : FILE* pFileWB = fopen()\n");
		return false;
	}

	Info forWrite;
	forWrite.hasMaterial = pFBX->m_MaterialLookUpVector->size() > 0 ? true : false;
	forWrite.meshCount = pFBX->m_MeshCount;
	forWrite.animStackCount = pFBX->m_AnimationStackVector->size();
	forWrite.poseCount = pFBX->m_PoseVector->size();
	forWrite.boneCount = pFBX->m_Skeleton->m_Bones.size();
	forWrite.clusterCount = pFBX->m_Skeleton->m_Clusters.size();
	unsigned int sumVertexCount = 0;
	for (unsigned int i = 0; i < pFBX->m_VerticesVector->size(); i++)
	{
		sumVertexCount += pFBX->m_VerticesVector->at(i)->m_Vertices.size();
	}
	forWrite.totalVertexCount = sumVertexCount;
	unsigned int sumPolygonCount = 0;
	for (unsigned int i = 0; i < pFBX->m_TrianglesVector->size(); i++)
	{
		sumPolygonCount += pFBX->m_TrianglesVector->at(i)->m_Triangles.size();
	}
	forWrite.totalPolygoncount = sumPolygonCount;


	if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Info), pFileWB) == -1)
	{
		printf("KSM.cpp SaveFBXInfo(FBX* pFBX, char* pFileName) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Info), pFileWB) == -1\n");
		return false;
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp SaveFBXInfo(FBX* pFBX, char* pFileName) : FILE* pFileRB = fopen()\n");
		return false;
	}

	Info* forRead;
	char buf[1024];

	if (fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB) <= 0)
	{
		printf("KSM.cpp : fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB) <= 0\n");
		return false;
	}

	char* readBuf = new char[sizeof(Info)];
	memcpy(readBuf, buf, sizeof(Info));
	forRead = reinterpret_cast<Info*>(readBuf);

	m_Info = *forRead;

	fclose(pFileRB);

	printf("forRead >>  hasMaterial : %d, meshCount : %d, animStackCount : %d, poseCount : %d, boneCount : %d, clusterCount : %d, totalVertexCount : %d, totalPolygoncount : %d\n",
		m_Info.hasMaterial,
		m_Info.meshCount,
		m_Info.animStackCount,
		m_Info.poseCount,
		m_Info.boneCount,
		m_Info.clusterCount,
		m_Info.totalVertexCount,
		m_Info.totalPolygoncount
	);
	/***** FBX ������ �ε� : ���� *****/

	return true;
}

bool KSM::SaveHasAnimation(FBX* pFBX, char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_HasAnimation_", ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveHasAnimation(FBX* pFBX, char* pFileName) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_HasAnimation->begin(); iter != pFBX->m_HasAnimation->end(); iter++)
	{
		bool forWrite = *iter;

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(bool), pFileWB) == -1)
		{
			printf("KSM.cpp SaveHasAnimation(FBX* pFBX, char* pFileName) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(bool), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SaveHasAnimation(FBX* pFBX, char* pFileName) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> hasAnimation : %s\n", *forRead ? "true" : "false");

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(bool);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(bool) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(bool), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SaveVertex(FBX* pFBX, char* pFileName, unsigned int meshCount)
{
	std::vector<Vertex> vertexVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Vertex_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveVertex(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_VerticesVector->at(meshCount)->m_Vertices.begin(); iter != pFBX->m_VerticesVector->at(meshCount)->m_Vertices.end(); iter++)
	{
		Vertex forWrite;
		forWrite.position = iter->m_Position;
		forWrite.normal = iter->m_Normal;
		forWrite.uv = iter->m_UV;

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Vertex), pFileWB) == -1)
		{
			printf("KSM.cpp SaveVertex(FBX* pFBX, char* pFileName, unsigned int meshCount) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Vertex), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SaveVertex(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

	// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> position : (%f, %f, %f), normal : (%f, %f, %f), uv : (%f, %f)\n",
			forRead->position.x,
			forRead->position.y,
			forRead->position.z,
			forRead->normal.x,
			forRead->normal.y,
			forRead->normal.z,
			forRead->uv.x,
			forRead->uv.y
		);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(Vertex);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(Vertex) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(Vertex), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SaveVertexAnim(FBX* pFBX, char* pFileName, unsigned int meshCount)
{
	std::vector<VertexAnim> vertexAnimVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_VertexAnim_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveVertexAnim(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_VerticesVector->at(meshCount)->m_Vertices.begin(); iter != pFBX->m_VerticesVector->at(meshCount)->m_Vertices.end(); iter++)
	{
		VertexAnim forWrite;
		forWrite.position = iter->m_Position;
		forWrite.normal = iter->m_Normal;
		forWrite.uv = iter->m_UV;
		for (int i = 0; i < 4; i++)
		{
			forWrite.boneIndex[i] = iter->m_VertexBlendingInfos.at(i).m_BlendingIndex;
			forWrite.boneWeight[i] = static_cast<float>(iter->m_VertexBlendingInfos.at(i).m_BlendingWeight);
		}

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(VertexAnim), pFileWB) == -1)
		{
			printf("KSM.cpp SaveVertexAnim(FBX* pFBX, char* pFileName, unsigned int meshCount) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(VertexAnim), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SaveVertexAnim(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> position : (%f, %f, %f), normal : (%f, %f, %f), uv : (%f, %f), bonIndex[4] : (%d, %d, %d, %d), boneWeight[4] : (%f, %f, %f, %f)\n",
			forRead->position.x,
			forRead->position.y,
			forRead->position.z,
			forRead->normal.x,
			forRead->normal.y,
			forRead->normal.z,
			forRead->uv.x,
			forRead->uv.y,
			forRead->boneIndex[0],
			forRead->boneIndex[1],
			forRead->boneIndex[2],
			forRead->boneIndex[3],
			forRead->boneWeight[0],
			forRead->boneWeight[1],
			forRead->boneWeight[2],
			forRead->boneWeight[3]
		);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(VertexAnim);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(VertexAnim) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(VertexAnim), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SavePolygon(FBX* pFBX, char* pFileName, unsigned int meshCount)
{
	std::vector<polygon> polygonVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Polygon_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SavePolygon(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_TrianglesVector->at(meshCount)->m_Triangles.begin(); iter != pFBX->m_TrianglesVector->at(meshCount)->m_Triangles.end(); iter++)
	{
		polygon forWrite;
		for (int i = 0; i < 3; i++)
		{
			forWrite.indices[i] = iter->m_Indices.at(i);
		}
		forWrite.materialIndex = iter->m_MaterialIndex;

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(polygon), pFileWB) == -1)
		{
			printf("KSM.cpp SavePolygon(FBX* pFBX, char* pFileName, unsigned int meshCount) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(polygon), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SavePolygon(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

	// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> indices[3] : (%d, %d, %d), materialIndex : %d\n",
			forRead->indices[0],
			forRead->indices[1],
			forRead->indices[2],
			forRead->materialIndex
		);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(polygon);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(polygon) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(polygon), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SaveMaterial(FBX* pFBX, char* pFileName, unsigned int meshCount)
{
	std::unordered_map<unsigned int, Material> materialVector;
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s", copy, "_Material_", meshCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveMaterial(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_MaterialLookUpVector->at(meshCount)->m_MaterialLookUp.begin(); iter != pFBX->m_MaterialLookUpVector->at(meshCount)->m_MaterialLookUp.end(); iter++)
	{
		Material forWrite;
		forWrite.index = iter->first;
		forWrite.lambert = iter->second->Lambert;
		forWrite.phong = iter->second->Phong;
		forWrite.ambient = iter->second->mAmbient;
		forWrite.diffuse = iter->second->mDiffuse;
		forWrite.emissive = iter->second->mEmissive;
		forWrite.specular = iter->second->mSpecular;
		forWrite.reflection = iter->second->mReflection;
		forWrite.shininess = static_cast<float>(iter->second->mShininess);
		forWrite.reflectionFactor = static_cast<float>(iter->second->mReflectionFactor);
		forWrite.specularPower = static_cast<float>(iter->second->mSpecularPower);
		forWrite.transparencyFactor = static_cast<float>(iter->second->mTransparencyFactor);

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Material), pFileWB) == -1)
		{
			printf("KSM.cpp SaveMaterial(FBX* pFBX, char* pFileName, unsigned int meshCount) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(Material), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SaveMaterial(FBX* pFBX, char* pFileName, unsigned int meshCount) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> index : %d, lambert : %s, phong : %s, ambient : (%f, %f, %f), diffuse : (%f, %f, %f), specular : (%f, %f, %f), shininess : %f\n",
			forRead->index,
			forRead->lambert ? "true" : "false",
			forRead->phong ? "true" : "false",
			forRead->ambient.x,
			forRead->ambient.y,
			forRead->ambient.z,
			forRead->diffuse.x,
			forRead->diffuse.y,
			forRead->diffuse.z,
			forRead->specular.x,
			forRead->specular.y,
			forRead->specular.z,
			forRead->shininess
		);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(Material);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(Material) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(Material), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SaveGlobalOffPosition(FBX* pFBX, char* pFileName)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%s", copy, "_GlobalOffPosition", ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveGlobalOffPosition(FBX* pFBX, char* pFileName) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iter = pFBX->m_GlobalOffPosition->begin(); iter != pFBX->m_GlobalOffPosition->end(); iter++)
	{
		GlobalOffPosition forWrite;
		forWrite.globalOffPosition = *iter;

		if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(GlobalOffPosition), pFileWB) == -1)
		{
			printf("KSM.cpp SaveGlobalOffPosition(FBX* pFBX, char* pFileName) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(GlobalOffPosition), pFileWB) == -1\n");
			return false;
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :v(FBX* pFBX, char* pFileName) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

					// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf(" <<globalOffPosition>>\n");
		printf("%f %f %f %f\n", tempGlobalOffPosition._11, tempGlobalOffPosition._12, tempGlobalOffPosition._13, tempGlobalOffPosition._14);
		printf("%f %f %f %f\n", tempGlobalOffPosition._21, tempGlobalOffPosition._22, tempGlobalOffPosition._23, tempGlobalOffPosition._24);
		printf("%f %f %f %f\n", tempGlobalOffPosition._31, tempGlobalOffPosition._32, tempGlobalOffPosition._33, tempGlobalOffPosition._34);
		printf("%f %f %f %f\n", tempGlobalOffPosition._41, tempGlobalOffPosition._42, tempGlobalOffPosition._43, tempGlobalOffPosition._44);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(GlobalOffPosition);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(GlobalOffPosition) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(GlobalOffPosition), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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

bool KSM::SaveFinalTransform(FBX* pFBX, char* pFileName, unsigned int meshCount, unsigned int animStackCount)
{
	std::list<char*> garbageCollector;

	/***** ��� �۾� : ���� *****/
	char copy[MAX_PATH];
	strncpy_s(copy, strlen(pFileName) + 1, pFileName, strlen(pFileName));
	char result[MAX_PATH];
	sprintf_s(result, "%s%s%d%s%d%s", copy, "_FinalTransform_", meshCount, "_", animStackCount, ".ksm");
	/***** ��� �۾� : ���� *****/


	/***** FBX ������ ���� : ���� *****/
	FILE* pFileWB = fopen(result, "wb");
	if (pFileWB == NULL)
	{
		printf("KSM.cpp :SaveFinalTransform(FBX* pFBX, char* pFileName, unsigned int meshCount, unsigned int animStackCount) : FILE* pFileWB = fopen()\n");
		return false;
	}

	for (auto iterCluster = pFBX->m_FinalTransform->at(meshCount).animVector.at(animStackCount).clustersUMAP.begin(); 
		iterCluster != pFBX->m_FinalTransform->at(meshCount).animVector.at(animStackCount).clustersUMAP.end();
		iterCluster++)
	{
		for (unsigned int i = 0; i < iterCluster->second.frameMatrix.size(); i++)
		{
			ClusterEachFrame forWrite;
			forWrite.index = iterCluster->first;
			forWrite.finalTransform = iterCluster->second.frameMatrix.at(i);

			printf("clusterIndex : %d, i : %d, size : %d\n", forWrite.index, i, iterCluster->second.frameMatrix.size());
			printf("struct Size : %d\n", sizeof(ClusterEachFrame));

			if (fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(ClusterEachFrame), pFileWB) == -1)
			{
				printf("KSM.cpp SaveFinalTransform(FBX* pFBX, char* pFileName, unsigned int meshCount, unsigned int animStackCount) : fwrite(reinterpret_cast<char*>(&forWrite), 1, sizeof(ClusterEachFrame), pFileWB) == -1\n");
				return false;
			}
		}
	}

	fclose(pFileWB);
	/***** FBX ������ ���� : ���� *****/


	/***** FBX ������ �ε� : ���� *****/
	FILE* pFileRB = fopen(result, "rb");
	if (pFileRB == NULL)
	{
		printf("KSM.cpp :SaveFinalTransform(FBX* pFBX, char* pFileName, unsigned int meshCount, unsigned int animStackCount) : FILE* pFileRB = fopen()\n");
		return false;
	}

	// ���ۿ� ����� �ӽõ�����<������, ������>�� �����ϴ� ť
	std::queue<std::pair<unsigned int, char*>> bufQueue;

	unsigned int bufLen = 0; // fread�� ���� ����
	char buf[1024]; // fread�� ���� ������ ������ ����

	// ���̻� ������ ������ 0���ϰ� ��ȯ�ǹǷ� ������ ������ ����
	while ((bufLen = fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), pFileRB)) > 0)
	{
		printf("bufLen : %d\n", bufLen);
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

		printf("forRead >> index : %d\n", forRead->index);
		printf(" <<finalTransform>>\n");
		printf("%f %f %f %f\n", tempFinalTransform._11, tempFinalTransform._12, tempFinalTransform._13, tempFinalTransform._14);
		printf("%f %f %f %f\n", tempFinalTransform._21, tempFinalTransform._22, tempFinalTransform._23, tempFinalTransform._24);
		printf("%f %f %f %f\n", tempFinalTransform._31, tempFinalTransform._32, tempFinalTransform._33, tempFinalTransform._34);
		printf("%f %f %f %f\n", tempFinalTransform._41, tempFinalTransform._42, tempFinalTransform._43, tempFinalTransform._44);

		// ���� �Ҵ��� readBuf ����
		delete[] readBuf;

		offset += sizeof(ClusterEachFrame);
		printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);

		if (offset + sizeof(ClusterEachFrame) > sumBufSize)
		{
			printf("offset + sizeof(Vertex) : %d, sumBufSize : %d\n", offset + sizeof(ClusterEachFrame), sumBufSize);
			if (bufQueue.empty())
				break;

			unsigned restBufSize = sumBufSize - offset;
			printf("restBufSize : %d\n", restBufSize);
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
			printf("offset : %d, sumBufSize : %d\n", offset, sumBufSize);
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