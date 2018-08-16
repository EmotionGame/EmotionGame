#pragma once

/////////////
// GLOBALS //
/////////////
const int TEXTURE_REPEAT = 8;

class Terrain
{
private:
	struct TerrainVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct TerrainHeightMapType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct TerrainVectorType
	{
		float x, y, z;
	};

public:
	Terrain();
	Terrain(const Terrain& other);
	~Terrain();

	bool Initialize(const char* pHeightMapFileName);
	void Shutdown();

	int GetVertexCount();
	void CopyVertexArray(void* pVertexList);

private:
	bool LoadHeightMap(const char* pFileName);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	void CalculateTextureCoordinates();

	bool InitializeBuffers();
	void ShutdownBuffers();

private:
	HWND m_hwnd;

	int m_terrainWidth = 0;
	int m_terrainHeight = 0;
	TerrainHeightMapType* m_heightMap = nullptr;
	int m_vertexCount = 0;
	TerrainVertexType* m_vertices = nullptr;
};