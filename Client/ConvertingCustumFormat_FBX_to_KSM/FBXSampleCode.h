#pragma once

/********** FBX SDK ���� �ڵ� viewScene���� ������ �ڵ� : ���� **********/
/***** GetPosition.cpp ���� : ���� *****/
FbxAMatrix GetGlobalPosition(FbxNode* pNode,
	const FbxTime& pTime,
	FbxPose* pPose = nullptr,
	FbxAMatrix* pParentGlobalPosition = nullptr);
FbxAMatrix GetPoseMatrix(FbxPose* pPose,
	int pNodeIndex);
FbxAMatrix GetGeometry(FbxNode* pNode);
/***** GetPosition.cpp ���� : ���� *****/


/***** DrawScene.cpp ���� : ���� *****/
void ComputeShapeDeformation(FbxMesh* pMesh,
	FbxTime& pTime,
	FbxAnimLayer * pAnimLayer,
	FbxVector4* pVertexArray);
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxCluster* pCluster,
	FbxAMatrix& pVertexTransformMatrix,
	FbxTime pTime,
	FbxPose* pPose);
void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose);
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose);
void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose);

void MatrixScale(FbxAMatrix& pMatrix, double pValue);
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue);
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix);
/***** DrawScene.cpp ���� : ���� *****/
/********** FBX SDK ���� �ڵ� viewScene���� ������ �ڵ� : ���� **********/