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
	void ProcessAnimationStackIteratively(FbxScene* pScene); // 애니메이션 스택들을 저장
	void ProcessCameraNodeVectorRecursively(FbxNode* pNode); // 카메라 노드들을 저장
	void ProcessPoseVectorIteratively(FbxScene* pScene); // 포즈들을 저장
	void LoadTextureIteratively(FbxScene* pScene); // 텍스처를 불러오는 코드 : 지금은 사용하지 않음

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

	void FillLightNodeVectorRecursively(FbxNode* pNode); // 라이트 저장 : 지금은 사용하지 않음

private:
	HWND m_hwnd;

public: // 후에 private로 변경해야 합니다.
	FbxManager * m_FBXManager = nullptr;	// FBX SDK 관리자, 프로그램에서 하나만 있어야 한다고 해서 static으로 했더니 FBX SDK에서 static으로 사용했다고 에러를 발생시킵니다.
	FbxScene* m_FBXScene = nullptr;			// Scene : FBX의 시작점

	/***** Model로 넘겨줄 데이터들 : 시작 *****/
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
	/***** Model로 넘겨줄 데이터들 : 종료 *****/
};