#pragma once

#include "LocalLight.h"
#include "Material.h"
#include "Utilities.h"

class FBX {
public:
	FBX();
	~FBX();

	bool Initialize(HWND hwnd, char* pFileName);
	void Shutdown();
	void ImportFBX();

private:
	void ProcessAnimationStackIteratively(FbxScene* pScene); // �ִϸ��̼� ���õ��� ����
	void ProcessCameraNodeVectorRecursively(FbxNode* pNode); // ī�޶� ������ ����
	void ProcessPoseVectorIteratively(FbxScene* pScene); // ������� ����
	void LoadTextureIteratively(FbxScene* pScene); // �ؽ�ó�� �ҷ����� �ڵ� : ������ ������� ����

	void ProcessSkeletonHierarchy(FbxNode* pRootNode);
	void ProcessSkeletonHierarchyRecursively(FbxNode* pNode, int inDepth, int myIndex, int inParentIndex);

	void ProcessGeometry(FbxNode* pNode);
	void ProcessControlPoints(FbxNode* pNode);
	void ProcessBones(FbxNode* pNode);
	unsigned int FindJointIndexUsingName(const std::string& BoneName, unsigned int clusterIndex);
	void ProcessMesh(FbxNode* pNode);
	void ReadUV(FbxMesh* pMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* pMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal);
	//void ReadBinormal(FbxMesh*, int, int, XMFLOAT3&);
	//void ReadTangent(FbxMesh*, int, int, XMFLOAT3&);
	
	void AssociateMaterialToMesh(FbxMesh* pMesh);
	void ProcessMaterials(FbxNode* pNode);
	void ProcessMaterialAttribute(FbxSurfaceMaterial* pMaterial, unsigned int inMaterialIndex);
	void ProcessMaterialTexture(FbxSurfaceMaterial* pFbxSurfaceMaterial, Material* pMaterial);

	void FillLightNodeVectorRecursively(FbxNode* pNode); // ����Ʈ ���� : ������ ������� ����

private:
	HWND m_hwnd;

public: // �Ŀ� private�� �����ؾ� �մϴ�.
	FbxManager * m_FBXManager = nullptr;	// FBX SDK ������, ���α׷����� �ϳ��� �־�� �Ѵٰ� �ؼ� static���� �ߴ��� FBX SDK���� static���� ����ߴٰ� ������ �߻���ŵ�ϴ�.
	FbxScene* m_FBXScene = nullptr;			// Scene : FBX�� ������

	/***** Model�� �Ѱ��� �����͵� : ���� *****/
	std::vector<Animation*>* m_AnimationStackVector = new std::vector<Animation*>;
	std::vector<FbxNode*>* m_CameraNodeVector = new std::vector<FbxNode*>;
	std::vector<FbxPose*>* m_PoseVector = new std::vector<FbxPose*>;

	Skeleton* m_Skeleton = new Skeleton;
	bool m_HasAnimation = true;

	std::unordered_map<unsigned int, CtrlPoint*> m_ControlPoints;

	unsigned int m_MeshCount = 0;
	std::vector<FbxNode*>* m_MeshNodeVector = new std::vector<FbxNode*>;
	std::vector<bool>* m_HasNormalVector = new std::vector<bool>;
	std::vector<bool>* m_HasUVVector = new std::vector<bool>;
	std::vector<unsigned int>* m_TriangleCountVector = new std::vector<unsigned int>;
	std::vector<TrianglePolygonVector*>* m_TrianglesVector = new std::vector<TrianglePolygonVector*>;
	std::vector<PNTIWVertexVector*>* m_VerticesVector = new std::vector<PNTIWVertexVector*>;

	std::vector<MaterialUnorderedMap*>* m_MaterialLookUpVector = new std::vector<MaterialUnorderedMap*>;

	std::vector<LightCache*>* m_LightCacheVector = new std::vector<LightCache*>;
	/***** Model�� �Ѱ��� �����͵� : ���� *****/
};