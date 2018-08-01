#include "stdafx.h"
#include "Texture.h"
#include "LocalLightShader.h"
#include "LocalLightAnimationShader.h"
#include "FBX.h"
#include "HID.h"
#include "Model.h"

#include <fstream>

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

/***** GetPosition.cpp 정의 : 종료 *****/
// Get the global position of the node for the current pose.
// If the specified node is not part of the pose or no pose is specified, get its
// global position at the current time.
FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
{
	FbxAMatrix lGlobalPosition;
	bool        lPositionFound = false;

	if (pPose)
	{
		int lNodeIndex = pPose->Find(pNode);

		if (lNodeIndex > -1)
		{
			// The bind pose is always a global matrix.
			// If we have a rest pose, we need to check if it is
			// stored in global or local space.
			if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
			{
				lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
			}
			else
			{
				// We have a local matrix, we need to convert it to
				// a global space matrix.
				FbxAMatrix lParentGlobalPosition;

				if (pParentGlobalPosition)
				{
					lParentGlobalPosition = *pParentGlobalPosition;
				}
				else
				{
					if (pNode->GetParent())
					{
						lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
					}
				}

				FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
				lGlobalPosition = lParentGlobalPosition * lLocalPosition;
			}

			lPositionFound = true;
		}
	}

	if (!lPositionFound)
	{
		// There is no pose entry for that node, get the current global position instead.

		// Ideally this would use parent global position and local position to compute the global position.
		// Unfortunately the equation 
		//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
		// does not hold when inheritance type is other than "Parent" (RSrs).
		// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
		lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
	}

	return lGlobalPosition;
}
// Get the matrix of the given pose
FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

	return lPoseMatrix;
}
// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix GetGeometry(FbxNode* pNode)
{
	const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}
/***** GetPosition.cpp 정의 : 종료 *****/


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

/***** DrawScene.cpp 정의 : 시작 *****/
// Deform the vertex array with the shapes contained in the mesh.
void ComputeShapeDeformation(FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, FbxVector4* pVertexArray)
{
	int lVertexCount = pMesh->GetControlPointsCount();

	FbxVector4* lSrcVertexArray = pVertexArray;
	FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
	memcpy(lDstVertexArray, pVertexArray, lVertexCount * sizeof(FbxVector4));

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for (int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if (lChannel)
			{
				// Get the percentage of influence on this channel.
				FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
				if (!lFCurve) continue;
				double lWeight = lFCurve->Evaluate(pTime);

				/*
				If there is only one targetShape on this channel, the influence is easy to calculate:
				influence = (targetShape - baseGeometry) * weight * 0.01
				dstGeometry = baseGeometry + influence

				But if there are more than one targetShapes on this channel, this is an in-between
				blendshape, also called progressive morph. The calculation of influence is different.

				For example, given two in-between targets, the full weight percentage of first target
				is 50, and the full weight percentage of the second target is 100.
				When the weight percentage reach 50, the base geometry is already be fully morphed
				to the first target shape. When the weight go over 50, it begin to morph from the
				first target shape to the second target shape.

				To calculate influence when the weight percentage is 25:
				1. 25 falls in the scope of 0 and 50, the morphing is from base geometry to the first target.
				2. And since 25 is already half way between 0 and 50, so the real weight percentage change to
				the first target is 50.
				influence = (firstTargetShape - baseGeometry) * (25-0)/(50-0) * 100
				dstGeometry = baseGeometry + influence

				To calculate influence when the weight percentage is 75:
				1. 75 falls in the scope of 50 and 100, the morphing is from the first target to the second.
				2. And since 75 is already half way between 50 and 100, so the real weight percentage change
				to the second target is 50.
				influence = (secondTargetShape - firstTargetShape) * (75-50)/(100-50) * 100
				dstGeometry = firstTargetShape + influence
				*/

				// Find the two shape indices for influence calculation according to the weight.
				// Consider index of base geometry as -1.

				int lShapeCount = lChannel->GetTargetShapeCount();
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();

				// Find out which scope the lWeight falls in.
				int lStartIndex = -1;
				int lEndIndex = -1;
				for (int lShapeIndex = 0; lShapeIndex < lShapeCount; ++lShapeIndex)
				{
					if (lWeight > 0 && lWeight <= lFullWeights[0])
					{
						lEndIndex = 0;
						break;
					}
					if (lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex + 1])
					{
						lStartIndex = lShapeIndex;
						lEndIndex = lShapeIndex + 1;
						break;
					}
				}

				FbxShape* lStartShape = NULL;
				FbxShape* lEndShape = NULL;
				if (lStartIndex > -1)
				{
					lStartShape = lChannel->GetTargetShape(lStartIndex);
				}
				if (lEndIndex > -1)
				{
					lEndShape = lChannel->GetTargetShape(lEndIndex);
				}

				//The weight percentage falls between base geometry and the first target shape.
				if (lStartIndex == -1 && lEndShape)
				{
					double lEndWeight = lFullWeights[0];
					// Calculate the real weight.
					lWeight = (lWeight / lEndWeight) * 100;
					// Initialize the lDstVertexArray with vertex of base geometry.
					memcpy(lDstVertexArray, lSrcVertexArray, lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the mesh vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
				//The weight percentage falls between two target shapes.
				else if (lStartShape && lEndShape)
				{
					double lStartWeight = lFullWeights[lStartIndex];
					double lEndWeight = lFullWeights[lEndIndex];
					// Calculate the real weight.
					lWeight = ((lWeight - lStartWeight) / (lEndWeight - lStartWeight)) * 100;
					// Initialize the lDstVertexArray with vertex of the previous target shape geometry.
					memcpy(lDstVertexArray, lStartShape->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the previous shape vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lStartShape->GetControlPoints()[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	memcpy(pVertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

	delete[] lDstVertexArray;
}
//Compute the transform matrix that the cluster will transform the vertex.
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxCluster* pCluster,
	FbxAMatrix& pVertexTransformMatrix,
	FbxTime pTime,
	FbxPose* pPose)
{
	FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	FbxAMatrix lReferenceGlobalInitPosition;
	FbxAMatrix lReferenceGlobalCurrentPosition;
	FbxAMatrix lAssociateGlobalInitPosition;
	FbxAMatrix lAssociateGlobalCurrentPosition;
	FbxAMatrix lClusterGlobalInitPosition;
	FbxAMatrix lClusterGlobalCurrentPosition;

	FbxAMatrix lReferenceGeometry;
	FbxAMatrix lAssociateGeometry;
	FbxAMatrix lClusterGeometry;

	FbxAMatrix lClusterRelativeInitPosition;
	FbxAMatrix lClusterRelativeCurrentPositionInverse;

	if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
	{
		pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
		// Geometric transform of the model
		lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = pGlobalPosition;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = GetGeometry(pCluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
			lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = pGlobalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}
// Deform the vertex array in classic linear way.
void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

		int lClusterCount = lSkinDeformer->GetClusterCount();

		for (int lClusterIndex = 0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k)
			{
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
				{
					continue;
				}

				// Compute the influence of the link on the vertex.
				FbxAMatrix lInfluence = lVertexTransformMatrix;
				MatrixScale(lInfluence, lWeight);

				if (lClusterMode == FbxCluster::eAdditive)
				{
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
					lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					MatrixAdd(lClusterDeformation[lIndex], lInfluence);

					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex			
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++)
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeight = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeight != 0.0)
		{
			lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeight;
			}
			if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeight);
				lDstVertex += lSrcVertex;
			}
		}
	}

	delete[] lClusterDeformation;
	delete[] lClusterWeight;
}
// Deform the vertex array in Dual Quaternion Skinning way.
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	for (int lSkinIndex = 0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for (int lClusterIndex = 0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k)
			{
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if (lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if (lSign >= 0.0)
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++)
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeightSum = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeightSum != 0.0)
		{
			lDQClusterDeformation[i].Normalize();
			lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeightSum;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeightSum);
				lDstVertex += lSrcVertex;
			}
		}
	}

	delete[] lDQClusterDeformation;
	delete[] lClusterWeight;
}
// Deform the vertex array according to the links contained in the mesh and the skinning type.
void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition,
	FbxMesh* pMesh,
	FbxTime& pTime,
	FbxVector4* pVertexArray,
	FbxPose* pPose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if (lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if (lSkinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if (lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = pMesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
		ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
		for (int lBWIndex = 0; lBWIndex < lBlendWeightsCount; ++lBWIndex)
		{
			double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
			pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
		}
	}
}

// Scale all the elements of a matrix.
void MatrixScale(FbxAMatrix& pMatrix, double pValue)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pMatrix[i][j] *= pValue;
		}
	}
}
// Add a value to all the elements in the diagonal of the matrix.
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
	pMatrix[0][0] += pValue;
	pMatrix[1][1] += pValue;
	pMatrix[2][2] += pValue;
	pMatrix[3][3] += pValue;
}
// Sum two matrices element by element.
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pDstMatrix[i][j] += pSrcMatrix[i][j];
		}
	}
}
/***** DrawScene.cpp 정의 : 종료 *****/
/********** FBX SDK 샘플 코드 viewScene에서 가져온 코드 : 종료 **********/


Model::Model()
{
}

Model::Model(const Model& other)
{
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, char* pModelFileName, WCHAR* pTextureFileName,
	XMFLOAT3 modelcaling, XMFLOAT3 modelRotation, XMFLOAT3 modelTranslation, bool specularZero)
{
#ifdef _DEBUG
	printf("Start >> Model.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;
	m_HID = pHID;

	// 초기 월드 매트릭스 계산용
	m_ModelScaling = modelcaling;
	m_ModelRotation = modelRotation;
	m_ModelTranslation = modelTranslation;

	// 머터리얼 값들 초기화
	m_SpecularZero = specularZero;
	for (int i = 0; i < MATERIAL_SIZE; i++)
	{
		m_AmbientColor[i] = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		m_DiffuseColor[i] = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		m_SpecularPower[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		m_SpecularColor[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// 모델 데이터를 로드합니다.
	if (!LoadModel(pModelFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadModel(modelFilename)", L"Error", MB_OK);
		return false;
	}

	// 애니메이션 유무 확인
	if (!m_HasAnimation)
	{
		// 정점 및 인덱스 버퍼를 초기화합니다.
		if (!InitializeBuffers(pDevice))
		{
			MessageBox(m_hwnd, L"Model.cpp : InitializeBuffers(device)", L"Error", MB_OK);
			return false;
		}
	}
	else
	{
		// 본 최종 변환 초기화
		for (int i = 0; i < BONE_FINAL_TRANSFORM_SIZE; i++)
		{
			m_FinalTransform[i] = XMMatrixIdentity();
		}

		// 애니메이션 정점 및 인덱스 버퍼를 초기화합니다.
		if (!InitializeAnimationBuffers(pDevice))
		{
			MessageBox(m_hwnd, L"Model.cpp : InitializeAnimationBuffers(device)", L"Error", MB_OK);
			return false;
		}
	}

	// 이 모델의 텍스처를 로드합니다.
	if (!LoadTexture(pDevice, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : LoadTexture(device, textureFilename)", L"Error", MB_OK);
		return false;
	}

	// 애니메이션 유무 확인
	if (!m_HasAnimation)
	{
		// LocalLightShader 객체 생성
		m_LocalLightShader = new LocalLightShader;
		if (!m_LocalLightShader)
		{
			MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader = new LocalLightShader;", L"Error", MB_OK);
			return false;
		}

		// LocalLightShader 객체 초기화
		if (!m_LocalLightShader->Initialize(pDevice, hwnd))
		{
			MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Initialize(device, hwnd)", L"Error", MB_OK);
			return false;
		}
	}
	else
	{
		// LocalLightAnimationShader 객체 생성
		m_LocalLightAnimationShader = new LocalLightAnimationShader;
		if (!m_LocalLightAnimationShader)
		{
			MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader = new m_LocalLightAnimationShader;", L"Error", MB_OK);
			return false;
		}

		// LocalLightAnimationShader 객체 초기화
		if (!m_LocalLightAnimationShader->Initialize(pDevice, hwnd))
		{
			MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Initialize(device, hwnd)", L"Error", MB_OK);
			return false;
		}

		// 애니메이션 시간 초기화
		mCurrentTime = m_AnimationStackVector->at(m_AnimStackIndex)->m_StartTime;
		mFrameTime.SetTime(0, 0, 0, 1, 0, m_Skeleton->m_Scene->GetGlobalSettings().GetTimeMode());
	}

#ifdef _DEBUG
	printf("Success >> Model.cpp : Initialize()\n");
#endif

	return true;
}

void Model::Shutdown()
{
	if (m_HasAnimation) // 애니메이션이 있다면
	{
		// LocalLightAnimationShader 객체 반환
		if (m_LocalLightAnimationShader)
		{
			m_LocalLightAnimationShader->Shutdown();
			delete m_LocalLightAnimationShader;
			m_LocalLightAnimationShader = nullptr;
		}
	}
	else // 애니메이션이 없다면
	{
		// LocalLightShader 객체 반환
		if (m_LocalLightShader)
		{
			m_LocalLightShader->Shutdown();
			delete m_LocalLightShader;
			m_LocalLightShader = nullptr;
		}
	}

	// 모델 텍스쳐를 반환합니다.
	ReleaseTexture();

	// 버텍스 및 인덱스 버퍼를 종료합니다.
	ShutdownBuffers();

	// 모델 반환
	if (m_Model) {
		switch (m_ModelFormat) {
		case FBX_FORMAT:
			static_cast<FBX*>(m_Model)->Shutdown();
			break;

		case OBJ_FORMAT:

			break;

		default:

			break;
		}

		m_Model = nullptr;
	}
}

bool Model::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float frameTime)
{
	// 월드 매트릭스 계산
	m_worldMatrix = CalculateWorldMatrix();

	// Mesh 개수만큼 반복
	for (unsigned int mc = 0; mc < m_MeshCount; mc++)
	{
		// PixelShader에 넘겨줄 머터리얼 저장
		if (!m_MaterialLookUpVector->empty()) // m_MaterialLookUpVector가 비어있지 않다면
		{
			for (auto iter = m_MaterialLookUpVector->at(mc)->m_MaterialLookUp.begin(); iter != m_MaterialLookUpVector->at(mc)->m_MaterialLookUp.end(); iter++)
			{
				m_AmbientColor[iter->first] = XMFLOAT4(iter->second->mAmbient.x, iter->second->mAmbient.y, iter->second->mAmbient.z, 1.0f);
				m_DiffuseColor[iter->first] = XMFLOAT4(iter->second->mDiffuse.x, iter->second->mDiffuse.y, iter->second->mDiffuse.z, 1.0f);
				m_SpecularPower[iter->first] = XMFLOAT4(static_cast<float>(iter->second->mShininess), 0.0f, 0.0f, 0.0f);
				m_SpecularColor[iter->first] = XMFLOAT4(iter->second->mSpecular.x, iter->second->mSpecular.y, iter->second->mSpecular.z, 1.0f);

				// 설정된 m_AmbientColor의 모든 값이 0.0f인 경우 보이지 않으므로 전부 0.05f로 변경
				if (m_AmbientColor[iter->first].x == 0.0f && m_AmbientColor[iter->first].y == 0.0f && m_AmbientColor[iter->first].z == 0.0f)
					m_AmbientColor[iter->first] = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);

				if (m_SpecularZero)
					m_SpecularColor[iter->first] = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			}
		}

		// 그리기를 준비하기 위해 그래픽 파이프 라인에 꼭지점과 인덱스 버퍼를 놓습니다.
		RenderBuffers(pDeviceContext, mc);

		// Default Pose 설정
		FbxPose* defaultPose = nullptr;
		if (!m_PoseVector->empty())
			defaultPose = m_PoseVector->at(0);

		FbxAMatrix pParentGlobalPosition; // 사용하지는 않습니다.


		// 애니메이션 유무에 따라 사용하는 쉐이더를 다르게 적용
		if (!m_HasAnimation) // 애니메이션이 없으면
		{
			XMMATRIX worldMatrix;

			FbxAMatrix lGlobalPosition = GetGlobalPosition(m_MeshNodeVector->at(mc), mCurrentTime, defaultPose, &pParentGlobalPosition);
			FbxAMatrix lGeometryOffset = GetGeometry(m_MeshNodeVector->at(mc));
			FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

			// 본래 변환을 적용하기 위해 lGlobalOffPosition 추가
			worldMatrix = ConvertFbxAMatrixtoXMMATRIX(lGlobalOffPosition) * m_worldMatrix;

			// LocalLightShader 쉐이더를 사용하여 모델을 렌더링합니다.
			if (!m_LocalLightShader->Render(pDeviceContext, m_VerticesVector->at(mc)->m_Vertices.size(), worldMatrix, viewMatrix, projectionMatrix,
				m_Texture->GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor))
			{
				MessageBox(m_hwnd, L"Model.cpp : m_LocalLightShader->Render", L"Error", MB_OK);
				return false;
			}
		}
		else // 애니메이션이 있으면
		{
			/***** 애니메이션 시간 설정 : 시작 *****/
			m_SumTime += frameTime;
			if (m_SumTime >= 33.333f) // 1초당 최대 30프레임이 진행되도록 설정
			{
				m_SumTime = 0.0f;

				mCurrentTime += mFrameTime;

				if (mCurrentTime > m_AnimationStackVector->at(m_AnimStackIndex)->m_EndTime)
				{
					mCurrentTime = m_AnimationStackVector->at(m_AnimStackIndex)->m_StartTime;
				}
			}
			/***** 애니메이션 시간 설정 : 종료 *****/

			XMMATRIX worldMatrix;

			// Default AnimStack 설정
			FbxAnimLayer* defaultAnimLayer = nullptr;
			defaultAnimLayer = m_AnimationStackVector->at(m_AnimStackIndex)->m_AnimStack->GetMember<FbxAnimLayer>();
			m_Skeleton->m_Scene->SetCurrentAnimationStack(m_AnimationStackVector->at(m_AnimStackIndex)->m_AnimStack);

			/***** Test : 시작 *****/
			//const bool lHasShape = m_MeshNodeVector->at(mc)->GetMesh()->GetShapeCount() > 0;
			//const bool lHasSkin = m_MeshNodeVector->at(mc)->GetMesh()->GetDeformerCount(FbxDeformer::eSkin) > 0;
			//const bool lHasDeformation = lHasShape || lHasSkin;

			//// 사용하지 않는 매트릭스
			//FbxAMatrix pParentGlobalPosition;

			//FbxAMatrix lGlobalPosition = GetGlobalPosition(m_MeshNodeVector->at(mc), mCurrentTime, defaultPose, &pParentGlobalPosition);
			//FbxAMatrix lGeometryOffset = GetGeometry(m_MeshNodeVector->at(mc));
			//FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

			//// 사용하지 않는 정점 배열
			//const int lVertexCount = m_MeshNodeVector->at(mc)->GetMesh()->GetControlPointsCount();
			//FbxVector4* lVertexArray = NULL;

			//if (lHasDeformation)
			//{
			//	lVertexArray = new FbxVector4[lVertexCount];
			//	memcpy(lVertexArray, m_MeshNodeVector->at(mc)->GetMesh()->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
			//}

			//if (lHasShape)
			//{
			//	// Deform the vertex array with the shapes.
			//	ComputeShapeDeformation(m_MeshNodeVector->at(mc)->GetMesh(), mCurrentTime, defaultAnimLayer, lVertexArray);
			//}

			////we need to get the number of clusters
			//const int lSkinCount = m_MeshNodeVector->at(mc)->GetMesh()->GetDeformerCount(FbxDeformer::eSkin);
			//int lClusterCount = 0;
			//for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
			//{
			//	lClusterCount += ((FbxSkin *)(m_MeshNodeVector->at(mc)->GetMesh()->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
			//}
			//if (lClusterCount)
			//{
			//	// Deform the vertex array with the skin deformer.
			//	ComputeSkinDeformation(lGlobalOffPosition, m_MeshNodeVector->at(mc)->GetMesh(), mCurrentTime, lVertexArray, defaultPose);
			//}

			//// 본래 변환을 적용하기 위해 lGlobalOffPosition 추가
			//worldMatrix = ConvertFbxAMatrixtoXMMATRIX(lGlobalOffPosition) * m_worldMatrix;

			//for (unsigned int i = 0; i < m_Skeleton->m_Clusters.size(); i++)
			//{
			//	// 최종 변환 저장
			//	m_FinalTransform[i] = ConvertFbxAMatrixtoXMMATRIX(g_MiddelTransform[i]);
			//}
			/***** Test : 종료 *****/


			/***** Bone 최종 변환 계산 : 시작 *****/
			FbxAMatrix lGlobalPosition = GetGlobalPosition(m_MeshNodeVector->at(mc), mCurrentTime, defaultPose, &pParentGlobalPosition);
			FbxAMatrix lGeometryOffset = GetGeometry(m_Skeleton->m_Nodes.at(mc));
			FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
			
			worldMatrix = ConvertFbxAMatrixtoXMMATRIX(lGlobalOffPosition) * m_worldMatrix;

			for (auto iter = m_Skeleton->m_Clusters.begin(); iter != m_Skeleton->m_Clusters.end(); iter++)
			{
				FbxAMatrix lVertexTransformMatrix;
				ComputeClusterDeformation(lGlobalOffPosition, m_Skeleton->m_Meshs.at(mc), iter->second, lVertexTransformMatrix, mCurrentTime, defaultPose);

				m_FinalTransform[iter->first] = ConvertFbxAMatrixtoXMMATRIX(lVertexTransformMatrix);
			}
			/*****  Bone 최종 변환 계산 : 종료 *****/

			// LocalLightAnimationShader 쉐이더를 사용하여 모델을 렌더링합니다.
			if (!m_LocalLightAnimationShader->Render(pDeviceContext, m_VerticesVector->at(mc)->m_Vertices.size(), worldMatrix, viewMatrix, projectionMatrix,
				m_Texture->GetTexture(), m_LightDirection, cameraPosition, m_AmbientColor, m_DiffuseColor, m_SpecularPower, m_SpecularColor,
				m_FinalTransform, m_HasAnimation))
			{
				MessageBox(m_hwnd, L"Model.cpp : m_LocalLightAnimationShader->Render", L"Error", MB_OK);
				return false;
			}
		}
	}

	return true;
}

void Model::MoveObejctToLookAt(XMFLOAT3 value)
{
	m_ModelTranslation.x += value.x * m_LootAt.x;
	//m_ModelTranslation.y += value.y * m_LootAt.y;
	m_ModelTranslation.z += value.z * m_LootAt.z;
}
void Model::MoveObjectToLookAtUp(XMFLOAT3 value)
{
	//m_ModelTranslation.x += value.x * m_Up.x;
	m_ModelTranslation.y += value.y * m_Up.y;
	//m_ModelTranslation.z += value.z * m_Up.z;
}
void Model::MoveObejctToLookAtSide(XMFLOAT3 value)
{
	m_ModelTranslation.x += value.x * m_Side.x;
	//m_ModelTranslation.y += value.y * m_Side.y;
	m_ModelTranslation.z += value.z * m_Side.z;
}
void Model::RotateObject(XMFLOAT3 value)
{
	if (m_ModelRotation.x >= 360.0f)
		m_ModelRotation.x += -360.0f;
	if (m_ModelRotation.x <= -360.0f)
		m_ModelRotation.x += 360.0f;

	if (m_ModelRotation.y >= 360.0f)
		m_ModelRotation.y += -360.0f;
	if (m_ModelRotation.y <= -360.0f)
		m_ModelRotation.y += 360.0f;

	if (m_ModelRotation.z >= 360.0f)
		m_ModelRotation.z += -360.0f;
	if (m_ModelRotation.z <= -360.0f)
		m_ModelRotation.z += 360.0f;

	m_ModelRotation.y += value.y;
	m_ModelRotation.z += value.z;


	// 각도 제한
	float result = m_ModelRotation.x += value.x;

	if (-m_limitAngle <= result && result <= m_limitAngle)
		m_ModelRotation.x += value.x;
	else if (result < -m_limitAngle)
		m_ModelRotation.x = -m_limitAngle;
	else if (m_limitAngle < result)
		m_ModelRotation.x = m_limitAngle;
}
void Model::PlayerControl(float frameTime)
{
	int mouseX, mouseY;
	m_HID->GetMouse_Keyboard()->GetDeltaMouse(mouseX, mouseY);

	float moveSpeed = 0.05f * frameTime;
	float rotateSpeed = 0.05f * frameTime;

	RotateObject(XMFLOAT3(rotateSpeed * mouseY, rotateSpeed * mouseX, 0.0f));

	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_W))
		MoveObejctToLookAt(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_S))
		MoveObejctToLookAt(XMFLOAT3(-moveSpeed, -moveSpeed, -moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_A))
		MoveObejctToLookAtSide(XMFLOAT3(-moveSpeed, -moveSpeed, -moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_D))
		MoveObejctToLookAtSide(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_E))
		MoveObjectToLookAtUp(XMFLOAT3(moveSpeed, moveSpeed, moveSpeed));
	if (m_HID->GetMouse_Keyboard()->IsKeyDown(DIK_Q))
		MoveObjectToLookAtUp(XMFLOAT3 (-moveSpeed, -moveSpeed, -moveSpeed));

	CalculateCameraPosition();
}

void Model::SetPosition(XMFLOAT3 position)
{
	m_ModelTranslation = position;
}
XMFLOAT3 Model::GetPosition()
{
	return m_ModelTranslation;
}
void Model::SetRotation(XMFLOAT3 rotation)
{
	m_ModelRotation = rotation;
}
XMFLOAT3 Model::GetRotation()
{
	return m_ModelRotation;
}
void Model::CalculateCameraPosition()
{
	m_cameraPosition.x = m_ModelTranslation.x + (5.0f * m_LootAt.x);
	m_cameraPosition.y = m_ModelTranslation.y + (6.5f * m_DefaultUp.y);
	m_cameraPosition.z = m_ModelTranslation.z + (5.0f * m_LootAt.z);
}
XMFLOAT3 Model::GetCameraPosition()
{
	return m_cameraPosition;
}

bool Model::IsActive()
{
	return m_ActiveFlag;
}

bool Model::LoadModel(char* pFileName)
{
	if (!CheckFormat(pFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : CheckFormat(filename)", L"Error", MB_OK);
		return false;
	}

	switch (m_ModelFormat) {
	case FBX_FORMAT:
		m_Model = new FBX;
		if (!m_Model)
		{
			MessageBox(m_hwnd, L"Model.cpp : m_Model = new FBX;", L"Error", MB_OK);
			return false;
		}
		static_cast<FBX*>(m_Model)->Initialize(m_hwnd, pFileName);
		m_AnimationStackVector = static_cast<FBX*>(m_Model)->m_AnimationStackVector;
		m_CameraNodeVector = static_cast<FBX*>(m_Model)->m_CameraNodeVector;
		m_PoseVector = static_cast<FBX*>(m_Model)->m_PoseVector;

		m_Skeleton = static_cast<FBX*>(m_Model)->m_Skeleton;
		m_HasAnimation = static_cast<FBX*>(m_Model)->m_HasAnimation;

		m_MeshCount = static_cast<FBX*>(m_Model)->m_MeshCount;
		m_MeshNodeVector = static_cast<FBX*>(m_Model)->m_MeshNodeVector;
		m_HasNormalVector = static_cast<FBX*>(m_Model)->m_HasNormalVector;
		m_HasUVVector = static_cast<FBX*>(m_Model)->m_HasUVVector;
		m_TriangleCountVector = static_cast<FBX*>(m_Model)->m_TriangleCountVector;
		m_TrianglesVector = static_cast<FBX*>(m_Model)->m_TrianglesVector;
		m_VerticesVector = static_cast<FBX*>(m_Model)->m_VerticesVector;

		m_MaterialLookUpVector = static_cast<FBX*>(m_Model)->m_MaterialLookUpVector;

		m_LightCacheVector = static_cast<FBX*>(m_Model)->m_LightCacheVector;

		break;

	case OBJ_FORMAT:

		break;

	default:

		break;
	}

	return true;
}

bool Model::InitializeBuffers(ID3D11Device* pDevice)
{
	// Mesh 개수만큼 반복
	for (unsigned int mc = 0; mc < m_MeshCount; mc++)
	{
		VertexType* m_vertices = new VertexType[m_VerticesVector->at(mc)->m_Vertices.size()];
		for (unsigned int i = 0; i < m_VerticesVector->at(mc)->m_Vertices.size(); i++)
		{
			m_vertices[i].position.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.x;
			m_vertices[i].position.y = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.y;
			m_vertices[i].position.z = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.z;

			m_vertices[i].texture.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_UV.x;
			m_vertices[i].texture.y = 1.0f - m_VerticesVector->at(mc)->m_Vertices.at(i).m_UV.y;

			m_vertices[i].normal.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.x;
			m_vertices[i].normal.y = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.y;
			m_vertices[i].normal.z = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.z;
		}

		unsigned int* m_indices = new unsigned int[m_TrianglesVector->at(mc)->m_Triangles.size() * 3];
		for (unsigned int i = 0; i < m_TrianglesVector->at(mc)->m_Triangles.size(); i++)
		{
			m_indices[3 * i] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(0);
			m_indices[3 * i + 1] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(1);
			m_indices[3 * i + 2] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(2);

			m_vertices[3 * i].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
			m_vertices[3 * i + 1].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
			m_vertices[3 * i + 2].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
		}

		// 정적 정점 버퍼의 구조체를 설정합니다.
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_VerticesVector->at(mc)->m_Vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = m_vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// 이제 정점 버퍼를 만듭니다.
		ID3D11Buffer* vertexBuffer;
		if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer)))
		{
			MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
			return false;
		}
		m_VertexBuffer->push_back(vertexBuffer);

		// 정적 인덱스 버퍼의 구조체를 설정합니다.
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_TrianglesVector->at(mc)->m_Triangles.size() * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = m_indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// 인덱스 버퍼를 생성합니다.
		ID3D11Buffer* indexBuffer;
		if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer)))
		{
			MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
			return false;
		}
		m_IndexBuffer->push_back(indexBuffer);

		// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
		delete[] m_vertices;
		m_vertices = nullptr;

		delete[] m_indices;
		m_indices = nullptr;
	}

	return true;
}

bool Model::InitializeAnimationBuffers(ID3D11Device* pDevice)
{
	// Mesh 개수만큼 반복
	for (unsigned int mc = 0; mc < m_MeshCount; mc++)
	{
		AnimationVertexType* m_vertices = new AnimationVertexType[m_VerticesVector->at(mc)->m_Vertices.size()];
		for (unsigned int i = 0; i < m_VerticesVector->at(mc)->m_Vertices.size(); i++)
		{
			m_vertices[i].position.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.x;
			m_vertices[i].position.y = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.y;
			m_vertices[i].position.z = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Position.z;

			m_vertices[i].normal.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.x;
			m_vertices[i].normal.y = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.y;
			m_vertices[i].normal.z = m_VerticesVector->at(mc)->m_Vertices.at(i).m_Normal.z;

			m_vertices[i].texture.x = m_VerticesVector->at(mc)->m_Vertices.at(i).m_UV.x;
			m_vertices[i].texture.y = 1.0f - m_VerticesVector->at(mc)->m_Vertices.at(i).m_UV.y;

			for (int j = 0; j < 4; j++)
			{
				m_vertices[i].blendingIndex[j] = m_VerticesVector->at(mc)->m_Vertices.at(i).m_VertexBlendingInfos.at(j).m_BlendingIndex;
				m_vertices[i].blendingWeight[j] = static_cast<float>(m_VerticesVector->at(mc)->m_Vertices.at(i).m_VertexBlendingInfos.at(j).m_BlendingWeight);
			}
		}

		unsigned int* m_indices = new unsigned int[m_TrianglesVector->at(mc)->m_Triangles.size() * 3];
		for (unsigned int i = 0; i < m_TrianglesVector->at(mc)->m_Triangles.size(); i++)
		{
			m_indices[3 * i] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(0);
			m_indices[3 * i + 1] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(1);
			m_indices[3 * i + 2] = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_Indices.at(2);

			m_vertices[3 * i].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
			m_vertices[3 * i + 1].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
			m_vertices[3 * i + 2].materialIndex = m_TrianglesVector->at(mc)->m_Triangles.at(i).m_MaterialIndex;
		}

		// 정적 정점 버퍼의 구조체를 설정합니다.
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(AnimationVertexType) * m_VerticesVector->at(mc)->m_Vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = m_vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// 이제 정점 버퍼를 만듭니다.
		ID3D11Buffer* animationVertexBuffer;
		if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &animationVertexBuffer)))
		{
			MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)", L"Error", MB_OK);
			return false;
		}
		m_AnimationVertexBuffer->push_back(animationVertexBuffer);

		// 정적 인덱스 버퍼의 구조체를 설정합니다.
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_TrianglesVector->at(mc)->m_Triangles.size() * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = m_indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// 인덱스 버퍼를 생성합니다.
		ID3D11Buffer* animationIndexBuffer;
		if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &animationIndexBuffer)))
		{
			MessageBox(m_hwnd, L"Model.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)", L"Error", MB_OK);
			return false;
		}
		m_AnimationIndexBuffer->push_back(animationIndexBuffer);

		// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
		delete[] m_vertices;
		m_vertices = nullptr;

		delete[] m_indices;
		m_indices = nullptr;
	}

	return true;
}

bool Model::LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName)
{
	// 텍스처 오브젝트를 생성한다.
	m_Texture = new Texture;
	if (!m_Texture)
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture = new Texture;", L"Error", MB_OK);
		return false;
	}

	// 텍스처 오브젝트를 초기화한다.
	if (!m_Texture->Initialize(pDevice, m_hwnd, pFileName))
	{
		MessageBox(m_hwnd, L"Model.cpp : m_Texture->Initialize(device, filename)", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Model::RenderBuffers(ID3D11DeviceContext* pDeviceContext, unsigned int i)
{
	if (!m_HasAnimation)
	{
		// 정점 버퍼의 단위와 오프셋을 설정합니다.
		UINT stride = sizeof(VertexType);
		UINT offset = 0;

		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer->at(i), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_IndexBuffer->at(i), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		// 정점 버퍼의 단위와 오프셋을 설정합니다.
		UINT stride = sizeof(AnimationVertexType);
		UINT offset = 0;

		// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetVertexBuffers(0, 1, &m_AnimationVertexBuffer->at(i), &stride, &offset);

		// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
		pDeviceContext->IASetIndexBuffer(m_AnimationIndexBuffer->at(i), DXGI_FORMAT_R32_UINT, 0);

		// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Model::ReleaseTexture()
{
	// 텍스처 오브젝트를 릴리즈한다.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}

void Model::ShutdownBuffers()
{
	if (m_HasAnimation)
	{
		for (auto iter = m_AnimationIndexBuffer->begin(); iter < m_AnimationIndexBuffer->end(); iter++)
		{
			(*iter)->Release();
			*iter = nullptr;
		}
		m_AnimationIndexBuffer->clear();
		delete m_AnimationIndexBuffer;
		m_AnimationIndexBuffer = nullptr;

		for (auto iter = m_AnimationVertexBuffer->begin(); iter < m_AnimationVertexBuffer->end(); iter++)
		{
			(*iter)->Release();
			*iter = nullptr;
		}
		m_AnimationVertexBuffer->clear();
		delete m_AnimationVertexBuffer;
		m_AnimationVertexBuffer = nullptr;
	}
	else
	{
		for (auto iter = m_IndexBuffer->begin(); iter < m_IndexBuffer->end(); iter++)
		{
			(*iter)->Release();
			*iter = nullptr;
		}
		m_IndexBuffer->clear();
		delete m_IndexBuffer;
		m_IndexBuffer = nullptr;

		for (auto iter = m_VertexBuffer->begin(); iter < m_VertexBuffer->end(); iter++)
		{
			(*iter)->Release();
			*iter = nullptr;
		}
		m_VertexBuffer->clear();
		delete m_VertexBuffer;
		m_VertexBuffer = nullptr;
	}
}

bool Model::CheckFormat(char* pFileName) {
	if (Last4strcmp(pFileName, ".fbx"))
	{
		m_ModelFormat = FBX_FORMAT;
		return true;
	}
	if (Last4strcmp(pFileName, ".obj"))
	{
		m_ModelFormat = OBJ_FORMAT;
		return true;
	}

	// 지원하는 포맷이 없으면 false 반환
	return false;
}
// 확장자 비교용 함수
bool Model::Last4strcmp(const char* pFileName, const char* pLast4FileName) {
	int filenameLen = strlen(pFileName);
	char last[5];

	last[0] = pFileName[filenameLen - 4];
	last[1] = pFileName[filenameLen - 3];
	last[2] = pFileName[filenameLen - 2];
	last[3] = pFileName[filenameLen - 1];
	last[4] = '\0';

	if (strcmp(last, pLast4FileName) == 0)
		return true;

	return false;
}

XMMATRIX Model::CalculateWorldMatrix()
{
	XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(m_ModelRotation.x * XM_RADIAN, m_ModelRotation.y * XM_RADIAN, m_ModelRotation.z * XM_RADIAN);
	XMMATRIX rM = XMMatrixRotationQuaternion(vQ);

	XMStoreFloat3(&m_LootAt, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultLootAt), rM));
	XMStoreFloat3(&m_Up, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultUp), rM));
	XMStoreFloat3(&m_Side, XMVector3TransformCoord(XMLoadFloat3(&m_DefaultSide), rM));

	XMMATRIX sM = XMMatrixScaling(m_ModelScaling.x, m_ModelScaling.y, m_ModelScaling.z);
	vQ = XMQuaternionRotationRollPitchYaw(0.0f, m_ModelRotation.y * XM_RADIAN, 0.0f);
	rM = XMMatrixRotationQuaternion(vQ);
	XMMATRIX tM = XMMatrixTranslation(m_ModelTranslation.x, m_ModelTranslation.y, m_ModelTranslation.z);

	return sM * rM * tM;
}

XMMATRIX Model::ConvertFbxAMatrixtoXMMATRIX(FbxAMatrix fbxAMatrix)
{
	XMFLOAT4X4 output;
	output._11 = static_cast<float>(fbxAMatrix.Get(0, 0));
	output._12 = static_cast<float>(fbxAMatrix.Get(0, 1));
	output._13 = static_cast<float>(fbxAMatrix.Get(0, 2));
	output._14 = static_cast<float>(fbxAMatrix.Get(0, 3));
	output._21 = static_cast<float>(fbxAMatrix.Get(1, 0));
	output._22 = static_cast<float>(fbxAMatrix.Get(1, 1));
	output._23 = static_cast<float>(fbxAMatrix.Get(1, 2));
	output._24 = static_cast<float>(fbxAMatrix.Get(1, 3));
	output._31 = static_cast<float>(fbxAMatrix.Get(2, 0));
	output._32 = static_cast<float>(fbxAMatrix.Get(2, 1));
	output._33 = static_cast<float>(fbxAMatrix.Get(2, 2));
	output._34 = static_cast<float>(fbxAMatrix.Get(2, 3));
	output._41 = static_cast<float>(fbxAMatrix.Get(3, 0));
	output._42 = static_cast<float>(fbxAMatrix.Get(3, 1));
	output._43 = static_cast<float>(fbxAMatrix.Get(3, 2));
	output._44 = static_cast<float>(fbxAMatrix.Get(3, 3));

	XMMATRIX gMatrix = XMLoadFloat4x4(&output);

	return gMatrix;
}