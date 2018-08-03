#pragma once

/********** FBX SDK 샘플 코드 viewScene에서 가져온 코드 : 시작 **********/
/***** GetPosition.cpp 선언 : 시작 *****/
FbxAMatrix GetGlobalPosition(FbxNode* pNode,
	const FbxTime& pTime,
	FbxPose* pPose = nullptr,
	FbxAMatrix* pParentGlobalPosition = nullptr);
FbxAMatrix GetPoseMatrix(FbxPose* pPose,
	int pNodeIndex);
FbxAMatrix GetGeometry(FbxNode* pNode);
/***** GetPosition.cpp 선언 : 종료 *****/


/***** DrawScene.cpp 선언 : 시작 *****/
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
/***** DrawScene.cpp 선언 : 종료 *****/
/********** FBX SDK 샘플 코드 viewScene에서 가져온 코드 : 종료 **********/