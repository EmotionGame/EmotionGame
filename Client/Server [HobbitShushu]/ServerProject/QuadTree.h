#pragma once
#include "Terrain.h"
/////////////
// GLOBALS //
/////////////
const int MAX_TRIANGLES = 10000;

class QuadTree
{
private:
	struct QuadTreeVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct QuadTreeVectorType
	{
		float x, y, z;
	};

	struct QuadTreeNodeType
	{
		float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		QuadTreeVectorType* vertexArray;
		QuadTreeNodeType* nodes[4];
	};

public:
	QuadTree();
	QuadTree(const QuadTree& other);
	~QuadTree();

	bool Initialize(Terrain* pTerrain);
	void Shutdown();

	int GetDrawCount();
	bool GetHeightAtPosition(float positionX, float positionZ, float& rHeight);

private:
	void CalculateMeshDimensions(int vertexCount, float& rCenterX, float& rCenterZ, float& rMeshWidth);
	void CreateTreeNode(QuadTreeNodeType* pNode, float positionX, float positionZ, float width);
	int CountTriangles(float positionX, float positionZ, float width);
	bool IsTriangleContained(int index, float positionX, float positionZ, float width);
	void ReleaseNode(QuadTreeNodeType* pNode);

	void FindNode(QuadTreeNodeType* pNode, float x, float z, float& rHeight);
	bool CheckHeightOfTriangle(float x, float z, float& rHeight, float v0[3], float v1[3], float v2[3]);

private:
	int m_triangleCount = 0;
	int m_drawCount = 0;
	QuadTreeVertexType* m_vertexList = nullptr;
	QuadTreeNodeType* m_parentNode = nullptr;
};