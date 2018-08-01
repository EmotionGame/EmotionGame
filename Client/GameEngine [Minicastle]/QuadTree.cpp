#include "stdafx.h"
#include "Terrain.h"
#include "Frustum.h"
#include "TerrainShader.h"
#include "QuadTree.h"

QuadTree::QuadTree()
{
}

QuadTree::QuadTree(const QuadTree& other)
{
}

QuadTree::~QuadTree()
{
}

bool QuadTree::Initialize(Terrain* pTerrain, ID3D11Device* pDevice)
{
	float centerX = 0.0f;
	float centerZ = 0.0f;
	float width = 0.0f;

	// 지형 정점 배열의 정점 수를 가져옵니다.
	int vertexCount = pTerrain->GetVertexCount();

	// 정점리스트의 총 삼각형 수를 저장합니다.
	m_triangleCount = vertexCount / 3;

	// 모든 지형 정점을 포함하는 정점 배열을 생성합니다.
	m_vertexList = new QuadTreeVertexType[vertexCount];
	if (!m_vertexList)
	{
		return false;
	}

	// 지형 정점을 정점 목록에 복사합니다.
	pTerrain->CopyVertexArray((void*)m_vertexList);

	// 중심 x, z와 메쉬의 너비를 계산합니다.
	CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

	// 쿼드 트리의 부모 노드를 생성합니다.
	m_parentNode = new QuadTreeNodeType;
	if (!m_parentNode)
	{
		return false;
	}

	// 정점 목록 데이터와 메쉬 차원을 기반으로 쿼드 트리를 재귀 적으로 빌드합니다.
	CreateTreeNode(m_parentNode, centerX, centerZ, width, pDevice);

	// 쿼드 트리가 각 노드에 정점을 갖기 때문에 정점 목록을 놓습니다.
	if (m_vertexList)
	{
		delete[] m_vertexList;
		m_vertexList = 0;
	}

	return true;
}

void QuadTree::Shutdown()
{
	// 쿼드 트리 데이터를 재귀 적으로 해제합니다.
	if (m_parentNode)
	{
		ReleaseNode(m_parentNode);
		delete m_parentNode;
		m_parentNode = 0;
	}
}

void QuadTree::Render(Frustum* pFrustum, ID3D11DeviceContext* pDeviceContext, TerrainShader* pShader)
{
	// 이 프레임에 대해 그려지는 삼각형의 수를 초기화합니다.
	m_drawCount = 0;

	// 부모 노드에서 시작하여 트리 아래로 이동하여 보이는 각 노드를 렌더링합니다.
	RenderNode(m_parentNode, pFrustum, pDeviceContext, pShader);
}

int QuadTree::GetDrawCount()
{
	return m_drawCount;
}

bool QuadTree::GetHeightAtPosition(float positionX, float positionZ, float& rHeight)
{
	float meshMinX = m_parentNode->positionX - (m_parentNode->width / 2.0f);
	float meshMaxX = m_parentNode->positionX + (m_parentNode->width / 2.0f);

	float meshMinZ = m_parentNode->positionZ - (m_parentNode->width / 2.0f);
	float meshMaxZ = m_parentNode->positionZ + (m_parentNode->width / 2.0f);

	// 좌표가 실제로 다각형 위에 있는지 확인하십시오.
	if ((positionX < meshMinX) || (positionX > meshMaxX) || (positionZ < meshMinZ) || (positionZ > meshMaxZ))
	{
		return false;
	}

	// 이 위치에 대한 다각형을 포함하는 노드를 찾습니다.
	FindNode(m_parentNode, positionX, positionZ, rHeight);

	return true;
}

void QuadTree::CalculateMeshDimensions(int vertexCount, float& rCenterX, float& rCenterZ, float& rMeshWidth)
{
	// 메쉬의 중심 위치를 0으로 초기화합니다.
	rCenterX = 0.0f;
	rCenterZ = 0.0f;

	// 메쉬의 모든 정점을 합친다.
	for (int i = 0; i<vertexCount; i++)
	{
		rCenterX += m_vertexList[i].position.x;
		rCenterZ += m_vertexList[i].position.z;
	}

	// 그리고 메쉬의 중간 점을 찾기 위해 정점의 수로 나눕니다.
	rCenterX = rCenterX / (float)vertexCount;
	rCenterZ = rCenterZ / (float)vertexCount;

	// 메쉬의 최대 및 최소 크기를 초기화합니다.
	float maxWidth = 0.0f;
	float maxDepth = 0.0f;

	float minWidth = fabsf(m_vertexList[0].position.x - rCenterX);
	float minDepth = fabsf(m_vertexList[0].position.z - rCenterZ);

	// 모든 정점을 살펴보고 메쉬의 최대 너비와 최소 너비와 깊이를 찾습니다.
	for (int i = 0; i<vertexCount; i++)
	{
		float width = fabsf(m_vertexList[i].position.x - rCenterX);
		float depth = fabsf(m_vertexList[i].position.z - rCenterZ);

		if (width > maxWidth) { maxWidth = width; }
		if (depth > maxDepth) { maxDepth = depth; }
		if (width < minWidth) { minWidth = width; }
		if (depth < minDepth) { minDepth = depth; }
	}

	// 최소와 최대 깊이와 너비 사이의 절대 최대 값을 찾습니다.
	float maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	float maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	// 메쉬의 최대 직경을 계산합니다.
	rMeshWidth = max(maxX, maxZ) * 2.0f;
}

void QuadTree::CreateTreeNode(QuadTreeNodeType* pNode, float positionX, float positionZ, float width, ID3D11Device* pDevice)
{
	// 노드의 위치와 크기를 저장한다.
	pNode->positionX = positionX;
	pNode->positionZ = positionZ;
	pNode->width = width;

	// 노드의 삼각형 수를 0으로 초기화합니다.
	pNode->triangleCount = 0;

	//정점 및 인덱스 버퍼를 null로 초기화합니다.
	pNode->vertexBuffer = 0;
	pNode->indexBuffer = 0;

	// Initialize the vertex array to null.
	pNode->vertexArray = 0;

	// 이 노드의 자식 노드를 null로 초기화합니다.
	pNode->nodes[0] = 0;
	pNode->nodes[1] = 0;
	pNode->nodes[2] = 0;
	pNode->nodes[3] = 0;

	// 이 노드 안에 있는 삼각형 수를 센다.
	int numTriangles = CountTriangles(positionX, positionZ, width);

	// 사례 1: 이 노드에 삼각형이 없으면 비어있는 상태로 돌아가서 처리할 필요가 없습니다.
	if (numTriangles == 0)
	{
		return;
	}

	// 사례 2: 이 노드에 너무 많은 삼각형이 있는 경우 4 개의 동일한 크기의 더 작은 트리 노드로 분할합니다.
	if (numTriangles > MAX_TRIANGLES)
	{
		for (int i = 0; i<4; i++)
		{
			// 새로운 자식 노드에 대한 위치 오프셋을 계산합니다.
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			// 새 노드에 삼각형이 있는지 확인합니다.
			int count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			if (count > 0)
			{
				// 이 새 노드가있는 삼각형이있는 경우 자식 노드를 생성합니다.
				pNode->nodes[i] = new QuadTreeNodeType;

				// 이제이 새 자식 노드에서 시작하는 트리를 확장합니다.
				CreateTreeNode(pNode->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), pDevice);
			}
		}
		return;
	}

	// 사례 3: 이 노드가 비어 있지않고 그 노드의 삼각형 수가 최대 값보다 작으면 
	// 이 노드는 트리의 맨 아래에 있으므로 저장할 삼각형 목록을 생성합니다.
	pNode->triangleCount = numTriangles;

	// 정점의 수를 계산합니다.
	int vertexCount = numTriangles * 3;

	// 정점 배열을 생성합니다.
	QuadTreeVertexType* vertices = new QuadTreeVertexType[vertexCount];

	// 인덱스 배열을 생성합니다.
	unsigned long* indices = new unsigned long[vertexCount];

	// 정점 배열을 생성합니다.
	pNode->vertexArray = new QuadTreeVectorType[vertexCount];

	// 이 새로운 정점 및 인덱스 배열의 인덱스를 초기화합니다.
	int index = 0;

	// 정점 목록의 모든 삼각형을 살펴 봅니다.
	int vertexIndex = 0;
	for (int i = 0; i<m_triangleCount; i++)
	{
		// 삼각형이이 노드 안에 있으면 꼭지점 배열에 추가합니다.
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			// 지형 버텍스 목록에 인덱스를 계산합니다.
			vertexIndex = i * 3;

			// 정점 목록에서 이 삼각형의 세 꼭지점을 가져옵니다.
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;

			// 또한 정점 위치 정보를 노드 정점 배열에 저장합니다.
			pNode->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			pNode->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			pNode->vertexArray[index].z = m_vertexList[vertexIndex].position.z;

			// 인덱스 값을 증가합니다.
			index++;

			vertexIndex++;

			// 다음 요점에 대해서도 똑같이하십시오.
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			pNode->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			pNode->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			pNode->vertexArray[index].z = m_vertexList[vertexIndex].position.z;
			index++;

			vertexIndex++;

			// 다음 요점에 대해서도 똑같이하십시오.
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			pNode->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			pNode->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			pNode->vertexArray[index].z = m_vertexList[vertexIndex].position.z;
			index++;
		}
	}

	// 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(QuadTreeVertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 마침내 정점 버퍼를 생성합니다.
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &pNode->vertexBuffer);

	// 인덱스 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 하위 리소스 구조에 인덱스 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pNode->indexBuffer);

	// 이제 노드의 버퍼에 데이터가 저장되므로 꼭지점과 인덱스 배열을 해제합니다.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

int QuadTree::CountTriangles(float positionX, float positionZ, float width)
{
	// 카운트를 0으로 초기화한다.
	int count = 0;

	// 전체 메쉬의 모든 삼각형을 살펴보고 어떤 노드가 이 노드 안에 있어야 하는지 확인합니다.
	for (int i = 0; i<m_triangleCount; i++)
	{
		// 삼각형이 노드 안에 있으면 1씩 증가시킵니다.
		if (IsTriangleContained(i, positionX, positionZ, width) == true)
		{
			count++;
		}
	}

	return count;
}

bool QuadTree::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	// 이 노드의 반경을 계산합니다.
	float radius = width / 2.0f;

	// 인덱스를 정점 목록으로 가져옵니다.
	int vertexIndex = index * 3;

	// 정점 목록에서 이 삼각형의 세 꼭지점을 가져옵니다.
	float x1 = m_vertexList[vertexIndex].position.x;
	float z1 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	float x2 = m_vertexList[vertexIndex].position.x;
	float z2 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	float x3 = m_vertexList[vertexIndex].position.x;
	float z3 = m_vertexList[vertexIndex].position.z;

	// 삼각형의 x 좌표의 최소값이 노드 안에 있는지 확인하십시오.
	float minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
	{
		return false;
	}

	// 삼각형의 x 좌표의 최대 값이 노드 안에 있는지 확인하십시오.
	float maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
	{
		return false;
	}

	// 삼각형의 z 좌표의 최소값이 노드 안에 있는지 확인하십시오.
	float minimumZ = min(z1, min(z2, z3));
	if (minimumZ >(positionZ + radius))
	{
		return false;
	}

	// 삼각형의 z 좌표의 최대 값이 노드 안에 있는지 확인하십시오.
	float maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
	{
		return false;
	}

	return true;
}

void QuadTree::ReleaseNode(QuadTreeNodeType* pNode)
{
	// 재귀적으로 트리 아래로 내려와 맨 아래 노드를 먼저 놓습니다.
	for (int i = 0; i<4; i++)
	{
		if (pNode->nodes[i] != 0)
		{
			ReleaseNode(pNode->nodes[i]);
		}
	}

	// 이 노드의 버텍스 버퍼를 해제한다.
	if (pNode->vertexBuffer)
	{
		pNode->vertexBuffer->Release();
		pNode->vertexBuffer = 0;
	}

	// 이 노드의 인덱스 버퍼를 해제합니다.
	if (pNode->indexBuffer)
	{
		pNode->indexBuffer->Release();
		pNode->indexBuffer = 0;
	}

	// 이 노드의 정점 배열을 해제합니다.
	if (pNode->vertexArray)
	{
		delete[] pNode->vertexArray;
		pNode->vertexArray = 0;
	}

	// 4개의 자식 노드를 해제합니다.
	for (int i = 0; i<4; i++)
	{
		if (pNode->nodes[i])
		{
			delete pNode->nodes[i];
			pNode->nodes[i] = 0;
		}
	}
}

void QuadTree::RenderNode(QuadTreeNodeType* pNode, Frustum* pFrustum, ID3D11DeviceContext* pDeviceContext, TerrainShader* pShader)
{
	// 노드를 볼 수 있는지, 높이는 쿼드 트리에서 중요하지 않은지 확인합니다.
	// 보이지 않는 경우 자식 중 하나도 트리 아래로 계속 진행할 수 없으며 속도가 증가한 곳입니다.
	if (!pFrustum->CheckCube(pNode->positionX, 0.0f, pNode->positionZ, (pNode->width / 2.0f)))
	{
		return;
	}

	// 볼 수 있는 경우 네 개의 자식 노드를 모두 확인하여 볼 ​​수 있는지 확인합니다.
	int count = 0;
	for (int i = 0; i<4; i++)
	{
		if (pNode->nodes[i] != 0)
		{
			count++;
			RenderNode(pNode->nodes[i], pFrustum, pDeviceContext, pShader);
		}
	}

	// 자식 노드가 있는 경우 부모 노드가 렌더링 할 삼각형을 포함하지 않으므로 계속할 필요가 없습니다.
	if (count != 0)
	{
		return;
	}

	// 그렇지 않으면 이 노드를 볼 수 있고 그 안에 삼각형이 있으면 이 삼각형을 렌더링합니다.

	// 정점 버퍼 보폭 및 오프셋을 설정합니다.
	unsigned int stride = sizeof(QuadTreeVertexType);
	unsigned int offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetVertexBuffers(0, 1, &pNode->vertexBuffer, &stride, &offset);

	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetIndexBuffer(pNode->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 이 꼭지점 버퍼에서 렌더링 되어야 하는 프리미티브 유형을 설정합니다. 이 경우에는 삼각형입니다.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 이 노드에서 인덱스의 수를 결정합니다.
	int indexCount = pNode->triangleCount * 3;

	// 지형 셰이더를 호출하여 이 노드의 다각형을 렌더링합니다.
	pShader->RenderShader(pDeviceContext, indexCount);

	// 이 프레임 동안 렌더링 된 폴리곤의 수를 늘립니다.
	m_drawCount += pNode->triangleCount;
}

void QuadTree::FindNode(QuadTreeNodeType* pNode, float x, float z, float& rHeight)
{
	// 이 노드의 크기를 계산합니다.
	float xMin = pNode->positionX - (pNode->width / 2.0f);
	float xMax = pNode->positionX + (pNode->width / 2.0f);

	float zMin = pNode->positionZ - (pNode->width / 2.0f);
	float zMax = pNode->positionZ + (pNode->width / 2.0f);

	// x 및 z 좌표가이 노드에 있는지 확인합니다. 그렇지 않으면 트리의이 부분을 탐색하지 않습니다.
	if ((x < xMin) || (x > xMax) || (z < zMin) || (z > zMax))
	{
		return;
	}

	// 좌표가 이 노드에 있으면 자식 노드가 있는지 먼저 확인합니다.
	int count = 0;

	for (int i = 0; i<4; i++)
	{
		if (pNode->nodes[i] != 0)
		{
			count++;
			FindNode(pNode->nodes[i], x, z, rHeight);
		}
	}

	// 자식 노드가 있는 경우 폴리곤이 자식중 하나에 있으므로 노드가 반환됩니다.
	if (count > 0)
	{
		return;
	}

	float vertex1[3] = { 0.0f, 0.0f, 0.0f };
	float vertex2[3] = { 0.0f, 0.0f, 0.0f };
	float vertex3[3] = { 0.0f, 0.0f, 0.0f };

	// 자식이 없으면 다각형이이 노드에 있어야합니다. 이 노드의 모든 다각형을 확인하여 찾습니다.
	// 우리가 찾고있는 폴리곤의 높이.
	for (int i = 0; i < pNode->triangleCount; i++)
	{
		int index = i * 3;
		vertex1[0] = pNode->vertexArray[index].x;
		vertex1[1] = pNode->vertexArray[index].y;
		vertex1[2] = pNode->vertexArray[index].z;

		index++;
		vertex2[0] = pNode->vertexArray[index].x;
		vertex2[1] = pNode->vertexArray[index].y;
		vertex2[2] = pNode->vertexArray[index].z;

		index++;
		vertex3[0] = pNode->vertexArray[index].x;
		vertex3[1] = pNode->vertexArray[index].y;
		vertex3[2] = pNode->vertexArray[index].z;

		// 이것이 우리가 찾고있는 폴리곤인지 확인합니다.
		// 삼각형 인 경우 함수를 종료하고 높이가 호출 함수에 반환됩니다.
		if (CheckHeightOfTriangle(x, z, rHeight, vertex1, vertex2, vertex3))
		{
			return;
		}
	}
}

bool QuadTree::CheckHeightOfTriangle(float x, float z, float& rHeight, float v0[3], float v1[3], float v2[3])
{
	float startVector[3] = { 0.0f, 0.0f, 0.0f };
	float directionVector[3] = { 0.0f, 0.0f, 0.0f };
	float edge1[3] = { 0.0f, 0.0f, 0.0f };
	float edge2[3] = { 0.0f, 0.0f, 0.0f };
	float normal[3] = { 0.0f, 0.0f, 0.0f };
	float Q[3] = { 0.0f, 0.0f, 0.0f };
	float e1[3] = { 0.0f, 0.0f, 0.0f };
	float e2[3] = { 0.0f, 0.0f, 0.0f };
	float e3[3] = { 0.0f, 0.0f, 0.0f };
	float edgeNormal[3] = { 0.0f, 0.0f, 0.0f };
	float temp[3] = { 0.0f, 0.0f, 0.0f };

	// 전송중인 광선의 시작 위치.
	startVector[0] = x;
	startVector[1] = 0.0f;
	startVector[2] = z;

	// 광선이 투영되는 방향입니다.
	directionVector[0] = 0.0f;
	directionVector[1] = -1.0f;
	directionVector[2] = 0.0f;

	// 주어진 세 점으로부터 두 모서리를 계산합니다.
	edge1[0] = v1[0] - v0[0];
	edge1[1] = v1[1] - v0[1];
	edge1[2] = v1[2] - v0[2];

	edge2[0] = v2[0] - v0[0];
	edge2[1] = v2[1] - v0[1];
	edge2[2] = v2[2] - v0[2];

	// 두 모서리에서 삼각형의 법선을 계산합니다.
	normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
	normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
	normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

	float magnitude = (float)sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));
	normal[0] = normal[0] / magnitude;
	normal[1] = normal[1] / magnitude;
	normal[2] = normal[2] / magnitude;

	// 원점에서 평면까지의 거리를 구합니다.
	float D = ((-normal[0] * v0[0]) + (-normal[1] * v0[1]) + (-normal[2] * v0[2]));

	// 방정식의 분모를 구하십시오.
	float denominator = ((normal[0] * directionVector[0]) + (normal[1] * directionVector[1]) + (normal[2] * directionVector[2]));

	// 결과가 0에 너무 가까워지지 않도록하여 0으로 나누는 것을 방지하십시오.
	if (fabs(denominator) < 0.0001f)
	{
		return false;
	}

	// 방정식의 분자를 구합니다.
	float numerator = -1.0f * (((normal[0] * startVector[0]) + (normal[1] * startVector[1]) + (normal[2] * startVector[2])) + D);

	// 삼각형과 교차하는 위치를 계산합니다.
	float t = numerator / denominator;

	// 교차 벡터를 찾습니다.
	Q[0] = startVector[0] + (directionVector[0] * t);
	Q[1] = startVector[1] + (directionVector[1] * t);
	Q[2] = startVector[2] + (directionVector[2] * t);

	// 삼각형의 세 모서리를 찾습니다.
	e1[0] = v1[0] - v0[0];
	e1[1] = v1[1] - v0[1];
	e1[2] = v1[2] - v0[2];

	e2[0] = v2[0] - v1[0];
	e2[1] = v2[1] - v1[1];
	e2[2] = v2[2] - v1[2];

	e3[0] = v0[0] - v2[0];
	e3[1] = v0[1] - v2[1];
	e3[2] = v0[2] - v2[2];

	// 첫 번째 가장자리의 법선을 계산합니다.
	edgeNormal[0] = (e1[1] * normal[2]) - (e1[2] * normal[1]);
	edgeNormal[1] = (e1[2] * normal[0]) - (e1[0] * normal[2]);
	edgeNormal[2] = (e1[0] * normal[1]) - (e1[1] * normal[0]);

	// 행렬이 내부, 외부 또는 직접 가장자리에 있는지 결정하기 위해 행렬식을 계산합니다.
	temp[0] = Q[0] - v0[0];
	temp[1] = Q[1] - v0[1];
	temp[2] = Q[2] - v0[2];

	float determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// 외부에 있는지 확인하십시오.
	if (determinant > 0.001f)
	{
		return false;
	}

	// 두 번째 가장자리의 법선을 계산합니다.
	edgeNormal[0] = (e2[1] * normal[2]) - (e2[2] * normal[1]);
	edgeNormal[1] = (e2[2] * normal[0]) - (e2[0] * normal[2]);
	edgeNormal[2] = (e2[0] * normal[1]) - (e2[1] * normal[0]);

	// 행렬이 내부, 외부 또는 직접 가장자리에 있는지 결정하기 위해 행렬식을 계산합니다.
	temp[0] = Q[0] - v1[0];
	temp[1] = Q[1] - v1[1];
	temp[2] = Q[2] - v1[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// 외부에 있는지 확인하십시오.
	if (determinant > 0.001f)
	{
		return false;
	}

	// 세 번째 가장자리의 법선을 계산합니다.
	edgeNormal[0] = (e3[1] * normal[2]) - (e3[2] * normal[1]);
	edgeNormal[1] = (e3[2] * normal[0]) - (e3[0] * normal[2]);
	edgeNormal[2] = (e3[0] * normal[1]) - (e3[1] * normal[0]);

	// 행렬이 내부, 외부 또는 직접 가장자리에 있는지 결정하기 위해 행렬식을 계산합니다.
	temp[0] = Q[0] - v2[0];
	temp[1] = Q[1] - v2[1];
	temp[2] = Q[2] - v2[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// 외부에 있는지 확인하십시오.
	if (determinant > 0.001f)
	{
		return false;
	}

	// 이제 우리 높이가 있습니다.
	rHeight = Q[1];

	return true;
}