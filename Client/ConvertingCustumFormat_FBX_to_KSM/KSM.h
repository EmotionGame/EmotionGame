#pragma once

class FBX;

class KSM
{
private:
	struct Info
	{
		bool hasMaterial = 0;
		unsigned int meshCount = 0;
		unsigned int animStackCount = 0;
		unsigned int poseCount = 0;
		unsigned int boneCount = 0;
		unsigned int clusterCount = 0;
		unsigned int totalVertexCount = 0;
		unsigned int totalPolygoncount = 0;
	};
	struct Vertex
	{
		XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);

	};
	struct VertexAnim
	{
		XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);
		unsigned int boneIndex[4] = { 0, 0, 0, 0 };
		float boneWeight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};
	struct polygon
	{
		unsigned int indices[3] = { 0, 0, 0 };
		unsigned int materialIndex = 0;
	};
	struct Material
	{
		unsigned int index = 0;
		bool lambert = false;
		bool phong = true;
		XMFLOAT3 ambient = XMFLOAT3(0.05f, 0.05f, 0.05f);
		XMFLOAT3 diffuse = XMFLOAT3(0.5f, 0.5f, 0.5f);
		XMFLOAT3 specular = XMFLOAT3(0.2f, 0.2f, 0.2f);
		XMFLOAT3 emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 reflection = XMFLOAT3(0.0f, 0.0f, 0.0f);
		float shininess = 64.0f;
		float reflectionFactor = 0.0f;
		float specularPower = 1.0f;
		float transparencyFactor = 0.0f;
	};
	struct GlobalOffPosition
	{
		XMMATRIX globalOffPosition = XMMatrixIdentity();;
	};
	struct ClusterEachFrame
	{
		unsigned int index = 0;
		XMMATRIX finalTransform = XMMatrixIdentity();;
	};

public:
	bool SaveFBX2KSM(FBX* pFBX, char* pFileName);

	bool SaveFBXInfo(FBX* pFBX, char* pFileName);
	bool SaveHasAnimation(FBX* pFBX, char* pFileName);
	bool SaveVertex(FBX* pFBX, char* pFileName, unsigned int meshCount);
	bool SaveVertexAnim(FBX* pFBX, char* pFileName, unsigned int meshCount);
	bool SavePolygon(FBX* pFBX, char* pFileName, unsigned int meshCount);
	bool SaveMaterial(FBX* pFBX, char* pFileName, unsigned int meshCount);
	bool SaveGlobalOffPosition(FBX* pFBX, char* pFileName);
	bool SaveFinalTransform(FBX* pFBX, char* pFileName, unsigned int meshCount, unsigned int animStackCount);

	Info m_Info;
	std::vector<bool> m_HasAnimation;
	std::unordered_map<unsigned int, std::vector<Vertex>> m_Vertex;
	std::unordered_map<unsigned int, std::vector<VertexAnim>> m_VertexAnim;
	std::vector<std::vector<polygon>> m_polygon;
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, Material>> m_Material;
	std::vector<XMMATRIX> m_GlobalOffPosition;

	struct ClusterMatrix
	{
		std::vector<XMMATRIX> finalTransform;
	};
	struct Animation
	{
		// std::unordered_map<클러스터 번호, 해당 클러스터의 각 프레임마다의 최종 변환을 저장하는 행렬>
		std::unordered_map<unsigned int, ClusterMatrix> m_Animation;
	};
	// std::unordered_map<메시 번호, 애니메이션 스택에 따른 각 애니메이션 벡터>
	std::unordered_map<unsigned int, std::vector<Animation>> m_Animations;
};