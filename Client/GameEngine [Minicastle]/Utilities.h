#pragma once

#include "Vertex.h"

struct BlendingIndexWeightPair
{
	unsigned int m_BlendingIndex;
	double m_BlendingWeight;

	BlendingIndexWeightPair() : 
		m_BlendingIndex(0),
		m_BlendingWeight(0)
	{}
};

// Each Control Point in FBX is basically a vertex
// in the physical world. For example, a cube has 8
// vertices(Control Points) in FBX
// Joints are associated with Control Points in FBX
// The mapping is one joint corresponding to 4
// Control Points(Reverse of what is done in a game engine)
// As a result, this struct stores a XMFLOAT3 and a 
// vector of joint indices
struct CtrlPoint
{
	XMFLOAT3 m_Position;
	std::vector<BlendingIndexWeightPair> m_BlendingInfo;

	CtrlPoint()
	{
		m_BlendingInfo.reserve(4);
	}
};

struct TrianglePolygon
{
	std::vector<unsigned int> m_Indices; 
	unsigned int m_MaterialIndex = 0; // 머터리얼이 없을 경우 0번 머터리얼을 사용하도록 초기화

	bool operator<(const TrianglePolygon& rhs)
	{
		return m_MaterialIndex < rhs.m_MaterialIndex;
	}
};
struct TrianglePolygonVector
{
	std::vector<TrianglePolygon> m_Triangles;
};

struct LightCache
{
	FbxLight::EType m_Type;
	float m_ColorRed;
	FbxAnimCurve* m_ColorRedAnimCurve;
	float m_ColorGreen;
	FbxAnimCurve* m_ColorGreenAnimCurve;
	float m_ColorBlue;
	FbxAnimCurve* m_ColorBlueAnimCurve;
	float m_ConeAngle;
	FbxAnimCurve* m_ConeAngleAnimCurve;
};

struct Animation
{
	FbxAnimStack* m_AnimStack;
	FbxString m_Name;
	FbxTime m_StartTime;
	FbxTime m_EndTime;
	FbxLongLong m_PlayLength;
};

struct Bone
{
	std::string m_Name;
	int m_ParentIndex;
	FbxAMatrix m_VertexTransformMatrix;

	Bone()
	{
		m_VertexTransformMatrix.SetIdentity();
		m_ParentIndex = -1;
	}
};
struct Skeleton
{
	FbxScene* m_Scene;
	std::vector<FbxNode*> m_Nodes;
	std::vector<FbxMesh*> m_Meshs;
	std::vector<Bone> m_Bones;
	std::unordered_map<unsigned int, FbxCluster*> m_Clusters;
};

class Utilities
{
public:

	// This function should be changed if exporting to another format
	static FbxAMatrix GetGeometryTransformation(FbxNode* inNode);

	static std::string GetFileName(const std::string& inInput);

	static std::string RemoveSuffix(const std::string& inInput);
};


