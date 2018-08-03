#include "stdafx.h"
#include "FBXSampleCode.h"
#include "FBX.h"

FBX::FBX()
{
}

FBX::~FBX()
{
}

bool FBX::Initialize(HWND hwnd, char* pFileName) {
	m_hwnd = hwnd;

	// SDK ������ ��ü ����
	m_FBXManager = FbxManager::Create();

	// ���� ��Ҹ� ���Ͽ��� �������ų� ���Ϸ� �������µ� ����ϴ� I/O���� ��ü �����
	/***** ����! FbxIOSettings�� Ŭ���ϸ� ���־� ��Ʃ����� ���� *****/
	m_IOSettings = FbxIOSettings::Create(m_FBXManager, IOSROOT);
	m_FBXManager->SetIOSettings(m_IOSettings);

	// �� ��������
	FbxImporter* importer = FbxImporter::Create(m_FBXManager, "");
	if (!importer->Initialize(pFileName, -1, m_FBXManager->GetIOSettings()))
	{
		m_IOSettings->Destroy();
		importer->Destroy();
		m_FBXManager->Destroy();

		MessageBox(m_hwnd, L"FBX.cpp : importer->Initialize(filename, -1, m_Manager->GetIOSettings())", L"Error", MB_OK);
		return false;
	}

	// SDK ������ ��ü�� ����Ͽ� �� ��ü ����ϴ�.
	m_FBXScene = FbxScene::Create(m_FBXManager, "m_Scene");
	// ���� ���� FBX ������ �ε��ϰ� ���ÿ� ����Ϸ��� ��쿡�� �� ���Ͽ� ���� ���� �����ؾ� �մϴ�.

	// ���� fbx ���� ������ �ε��մϴ�.
	if (!importer->Import(m_FBXScene)) // �ε忡 �����ϸ�
	{
		m_FBXScene->Destroy();
		m_IOSettings->Destroy();
		importer->Destroy();
		m_FBXManager->Destroy();

		MessageBox(m_hwnd, L"FBX.cpp : importer->Import(m_FBXScene)", L"Error", MB_OK);
		return false;
	}

	// ������ ä�����Ƿ� importer �Ҹ��մϴ�.
	importer->Destroy();

	// ��ǥ���� �����´�.
	FbxAxisSystem sceneAxisSystem = m_FBXScene->GetGlobalSettings().GetAxisSystem();

	// ���̷�ƮX�� LeftHanded�� �ؾ������� ����� ������ ���� �ʾ� RightHanded�� ����߽��ϴ�.
	FbxAxisSystem ourAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if (sceneAxisSystem != ourAxisSystem)
	{
		ourAxisSystem.ConvertScene(m_FBXScene);
	}

	// ���� �ý����� �����ϴ� �ڵ��� �� �����ϴ�.
	// ��������� �ʽ��ϴ�.
	// Convert Unit System to what is used in this example, if needed
	//FbxSystemUnit sceneSystemUnit = m_FBXScene->GetGlobalSettings().GetSystemUnit();
	//if (sceneSystemUnit.GetScaleFactor() != 1.0)
	//{
	//	//The unit in this example is centimeter.
	//	FbxSystemUnit::cm.ConvertScene(m_FBXScene);
	//}

	// �� ������ �ﰢ��ȭ �� �� �ִ� ��� ��带 �ﰢ��ȭ ��ŵ�ϴ�.
	// Tip) 3dsMax���� �̸� Triangulate üũ�ؼ� �ͽ���Ʈ�ϸ� ������ �����ϴ�.
	FbxGeometryConverter geometryConverter(m_FBXManager);
	geometryConverter.Triangulate(m_FBXScene, true);

	// ���� ����� �ε� ����
	ImportFBX();

	return true;
}

void FBX::Shutdown()
{
	if (m_LightCacheVector)
	{
		for (auto iter = m_LightCacheVector->begin(); iter != m_LightCacheVector->end(); iter++)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_LightCacheVector->clear();
		delete m_LightCacheVector;
		m_LightCacheVector = nullptr;
	}

	if (m_MaterialLookUpVector)
	{
		for (auto iter = m_MaterialLookUpVector->begin(); iter != m_MaterialLookUpVector->end(); iter++)
		{
			for (auto iter2 = (*iter)->m_MaterialLookUp.begin(); iter2 != (*iter)->m_MaterialLookUp.end(); iter2++)
			{
				delete (*iter2).second;
				(*iter2).second = nullptr;
			}

			delete *iter;
			*iter = nullptr;
		}
		m_MaterialLookUpVector->clear();
		delete m_MaterialLookUpVector;
		m_MaterialLookUpVector = nullptr;
	}

	if (m_VerticesVector)
	{
		for (auto iter = m_VerticesVector->begin(); iter != m_VerticesVector->end(); iter++)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_VerticesVector->clear();
		delete m_VerticesVector;
		m_VerticesVector = nullptr;
	}

	if (m_TrianglesVector)
	{
		for (auto iter = m_TrianglesVector->begin(); iter != m_TrianglesVector->end(); iter++)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_TrianglesVector->clear();
		delete m_TrianglesVector;
		m_TrianglesVector = nullptr;
	}

	if (m_TriangleCountVector)
	{
		m_TriangleCountVector->clear();
		delete m_TriangleCountVector;
		m_TriangleCountVector = nullptr;
	}

	if (m_HasUVVector)
	{
		m_HasUVVector->clear();
		delete m_HasUVVector;
		m_HasUVVector = nullptr;
	}

	if (m_HasNormalVector)
	{
		m_HasNormalVector->clear();
		delete m_HasNormalVector;
		m_HasNormalVector = nullptr;
	}

	if (m_MeshNodeVector)
	{
		m_MeshNodeVector->clear();
		delete m_MeshNodeVector;
		m_MeshNodeVector = nullptr;
	}

	if (m_Skeleton)
	{
		delete m_Skeleton;
		m_Skeleton = nullptr;
	}

	if (m_PoseVector)
	{
		m_PoseVector->clear();
		delete m_PoseVector;
		m_PoseVector = nullptr;
	}

	if (m_CameraNodeVector)
	{
		m_CameraNodeVector->clear();
		delete m_CameraNodeVector;
		m_CameraNodeVector = nullptr;
	}

	if (m_AnimationStackVector)
	{
		for (auto iter = m_AnimationStackVector->begin(); iter != m_AnimationStackVector->end(); iter++)
		{
			delete *iter;
			*iter = nullptr;
		}
		m_AnimationStackVector->clear();
		delete m_AnimationStackVector;
		m_AnimationStackVector = nullptr;
	}

	m_FBXScene->Destroy();
	m_IOSettings->Destroy();
	m_FBXManager->Destroy();
}

void FBX::ImportFBX()
{
	ProcessAnimationStackIteratively(m_FBXScene);
	ProcessCameraNodeVectorRecursively(m_FBXScene->GetRootNode());
	ProcessPoseVectorIteratively(m_FBXScene);
	LoadTextureIteratively(m_FBXScene);

	ProcessSkeletonHierarchy(m_FBXScene->GetRootNode());
	m_Skeleton->m_Scene = m_FBXScene;

	ProcessGeometry(m_FBXScene->GetRootNode());
}

void FBX::ProcessAnimationStackIteratively(FbxScene* pScene)
{
	// AnimStack �� ���� ����
	unsigned int animStackCount = m_FBXScene->GetSrcObjectCount<FbxAnimStack>();

	for (unsigned int i = 0; i < animStackCount; i++)
	{
		Animation* animStack = new Animation;

		animStack->m_AnimStack = m_FBXScene->GetSrcObject<FbxAnimStack>(i);
		animStack->m_Name = animStack->m_AnimStack->GetName();

		FbxTakeInfo* takeInfo = m_FBXScene->GetTakeInfo(animStack->m_Name);
		if (takeInfo)
		{
			animStack->m_StartTime = takeInfo->mLocalTimeSpan.GetStart();
			animStack->m_EndTime = takeInfo->mLocalTimeSpan.GetStop();
		}
		else
		{
			// Take the time line value
			FbxTimeSpan lTimeLineTimeSpan;
			m_FBXScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);

			animStack->m_StartTime = lTimeLineTimeSpan.GetStart();
			animStack->m_EndTime = lTimeLineTimeSpan.GetStop();
		}

		// ���� �𸣰����� �ϴ� ����
		animStack->m_PlayLength = animStack->m_EndTime.GetFrameCount(FbxTime::eFrames24) - animStack->m_StartTime.GetFrameCount(FbxTime::eFrames24) + 1;

		m_AnimationStackVector->push_back(animStack);
	}
}

void FBX::ProcessCameraNodeVectorRecursively(FbxNode* pNode)
{
	if (pNode)
	{
		if (pNode->GetNodeAttribute())
		{
			if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
			{
				m_CameraNodeVector->push_back(pNode);
			}
		}

		const int lCount = pNode->GetChildCount();
		for (int i = 0; i < lCount; i++)
		{
			ProcessCameraNodeVectorRecursively(pNode->GetChild(i));
		}
	}
}

void FBX::ProcessPoseVectorIteratively(FbxScene* pScene)
{
	const int lPoseCount = pScene->GetPoseCount();

	for (int i = 0; i < lPoseCount; ++i)
	{
		m_PoseVector->push_back(pScene->GetPose(i));
	}
}

// �ؽ�ó�� �ҷ����� �ڵ����� ������ ������� ����
void FBX::LoadTextureIteratively(FbxScene* pScene)
{
	// Load the textures into GPU, only for file texture now
	const int lTextureCount = pScene->GetTextureCount();
	for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
	{
		FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
		FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
		if (lFileTexture)
		{
			// ���⼭ �۾� ����
		}
	}
}

void FBX::ProcessSkeletonHierarchy(FbxNode* pRootNode)
{

	for (int childIndex = 0; childIndex < pRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = pRootNode->GetChild(childIndex);
		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	}
}

void FBX::ProcessSkeletonHierarchyRecursively(FbxNode* pNode, int inDepth, int myIndex, int inParentIndex)
{
	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone currBone;
		currBone.m_ParentIndex = inParentIndex;
		currBone.m_Name = pNode->GetName();
		m_Skeleton->m_Bones.push_back(currBone);
	}
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(pNode->GetChild(i), inDepth + 1, m_Skeleton->m_Bones.size(), myIndex);
	}
}

void FBX::ProcessGeometry(FbxNode* pNode)
{
	if (pNode->GetNodeAttribute())
	{
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessControlPoints(pNode);

			// �޽ð� �ִϸ��̼��� �ִ��� üũ
			ProcessBones(pNode);

			ProcessMesh(pNode);
			AssociateMaterialToMesh(pNode->GetMesh());
			ProcessMaterials(pNode);

			ProcessGlobalOffPosition(pNode);

			// �޽ð� �ִϸ��̼��� �ִ� ���
			if (m_HasAnimation->back())
			{
				ProcessFinalTransform(pNode);
			}

			m_MeshCount++;
			break;


		case FbxNodeAttribute::eLight:
			FillLightNodeVectorRecursively(pNode);
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		ProcessGeometry(pNode->GetChild(i));
	}
}

void FBX::ProcessControlPoints(FbxNode* pNode)
{
	FbxMesh* currMesh = pNode->GetMesh();
	unsigned int ctrlPointCount = currMesh->GetControlPointsCount();

	for (unsigned int i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* currCtrlPoint = new CtrlPoint();
		XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->m_Position = currPosition;
		m_ControlPoints[i] = currCtrlPoint;
	}
}

void FBX::ProcessBones(FbxNode* pNode)
{
	FbxMesh* currMesh = pNode->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount(FbxDeformer::eSkin);
	if (numOfDeformers)
	{
		m_HasAnimation->push_back(true);

		int lVertexCount = currMesh->GetControlPointsCount();

		// This geometry transform is something I cannot understand
		// I think it is from MotionBuilder
		// If you are using Maya for your models, 99% this is just an
		// identity matrix
		// But I am taking it into account anyways......
		FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(pNode);

		// A deformer is a FBX thing, which contains some clusters
		// A cluster contains a link, which is basically a joint
		// Normally, there is only one deformer in a mesh
		for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{

			// There are many types of deformers in Maya,
			// We are using only skins, so we see if this is a skin
			FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin)
			{
				continue;
			}

			// numOfClusters�� ���� �� ����
			unsigned int numOfClusters = currSkin->GetClusterCount();
			for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
			{
				FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);

				// Ŭ�������� ��ũ�� ������ �ǳ� ��
				if (!currCluster->GetLink())
					continue;

				std::string currJointName = currCluster->GetLink()->GetName();
				unsigned int currJointIndex = FindJointIndexUsingName(currJointName, clusterIndex);

				/***** *****/
				m_Skeleton->m_Clusters[currJointIndex] = currCluster;
				/***** *****/

				// ���⼭ �� ������ Bone Index�� Weight�� ����
				// Associate each joint with the control points it affects
				unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
				for (unsigned int i = 0; i < numOfIndices; ++i)
				{
					// ���� �޽��� ���� ������ �� ���� �� ����
					if (currCluster->GetControlPointIndices()[i] >= lVertexCount)
						continue;

					BlendingIndexWeightPair currBlendingIndexWeightPair;
					currBlendingIndexWeightPair.m_BlendingIndex = currJointIndex;
					currBlendingIndexWeightPair.m_BlendingWeight = currCluster->GetControlPointWeights()[i];
					m_ControlPoints[currCluster->GetControlPointIndices()[i]]->m_BlendingInfo.push_back(currBlendingIndexWeightPair);
				}
			}
		}

		// Some of the control points only have less than 4 joints
		// affecting them.
		// For a normal renderer, there are usually 4 joints
		// I am adding more dummy joints if there isn't enough
		BlendingIndexWeightPair currBlendingIndexWeightPair;
		currBlendingIndexWeightPair.m_BlendingIndex = 0;
		currBlendingIndexWeightPair.m_BlendingWeight = 0;
		for (auto itr = m_ControlPoints.begin(); itr != m_ControlPoints.end(); ++itr)
		{
			for (unsigned int i = itr->second->m_BlendingInfo.size(); i < 4; ++i)
			{
				itr->second->m_BlendingInfo.push_back(currBlendingIndexWeightPair);
			}
		}
	}
	else
	{
		m_HasAnimation->push_back(false);
	}
}

// �̸����� �ε����� ã�µ� ���ٸ� �׳� clusterIndex�� ���
unsigned int FBX::FindJointIndexUsingName(const std::string& BoneName, unsigned int clusterIndex)
{
	for (unsigned int i = 0; i < m_Skeleton->m_Bones.size(); ++i)
	{
		if (m_Skeleton->m_Bones.at(i).m_Name == BoneName)
		{
			return i;
		}
	}

	// ������ ���⼭ ���� ������ �׳� clusterIndex�� ���
	//throw std::exception("Skeleton information in FBX file is corrupted.");
	return clusterIndex;
}

void FBX::ProcessMesh(FbxNode* pNode)
{
	m_MeshNodeVector->push_back(pNode);

	FbxMesh* currMesh = pNode->GetMesh();

	m_HasNormalVector->push_back(currMesh->GetElementNormalCount() > 0);
	m_HasUVVector->push_back(currMesh->GetElementUVCount() > 0);

	m_TriangleCountVector->push_back(currMesh->GetPolygonCount());
	int vertexCounter = 0;

	TrianglePolygonVector* trianglePolygonVector = new TrianglePolygonVector;
	trianglePolygonVector->m_Triangles.reserve(m_TriangleCountVector->at(m_MeshCount));

	PNTIWVertexVector* pntiwVertexVector = new PNTIWVertexVector;

	for (unsigned int i = 0; i < m_TriangleCountVector->at(m_MeshCount); ++i)
	{
		XMFLOAT2 UV[3][2];
		XMFLOAT3 normal[3];
		//XMFLOAT3 binormal[3];
		//XMFLOAT3 tangent[3];

		TrianglePolygon currTriangle;
		trianglePolygonVector->m_Triangles.push_back(currTriangle);

		for (unsigned int j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = m_ControlPoints[ctrlPointIndex];

			// �븻�� ������
			if (m_HasNormalVector->back())
				ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);
			else
				normal[j] = XMFLOAT3(0.0f, 0.0f, 0.0f);

			// UV�� ������
			if (m_HasUVVector->back())
			{
				// We only have diffuse texture
				for (int k = 0; k < 1; ++k)
				{
					ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]);
				}
			}
			else
				UV[j][0] = XMFLOAT2(0.0f, 0.0f);

			PNTIWVertex temp;
			temp.m_Position = currCtrlPoint->m_Position;
			temp.m_Normal = normal[j];
			temp.m_UV = UV[j][0];

			// Copy the blending info from each control point
			// ��Ʈ�� ����Ʈ�� m_BlendingInfo�� �״�� ����
			for (unsigned int i = 0; i < currCtrlPoint->m_BlendingInfo.size(); ++i)
			{
				VertexBlendingInfo currBlendingInfo;
				currBlendingInfo.m_BlendingIndex = currCtrlPoint->m_BlendingInfo[i].m_BlendingIndex;
				currBlendingInfo.m_BlendingWeight = currCtrlPoint->m_BlendingInfo[i].m_BlendingWeight;
				temp.m_VertexBlendingInfos.push_back(currBlendingInfo);
			}
			// Sort the blending info so that later we can remove
			// duplicated vertices
			temp.SortBlendingInfoByWeight();

			pntiwVertexVector->m_Vertices.push_back(temp);
			trianglePolygonVector->m_Triangles.back().m_Indices.push_back(vertexCounter);
			++vertexCounter;
		}
	}

	m_TrianglesVector->push_back(trianglePolygonVector);
	m_VerticesVector->push_back(pntiwVertexVector);

	// Now m_ControlPoints has served its purpose
	// We can free its memory
	for (auto itr = m_ControlPoints.begin(); itr != m_ControlPoints.end(); ++itr)
	{
		delete itr->second;
	}
	m_ControlPoints.clear();
}

void FBX::ReadUV(FbxMesh* pMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV)
{
	FbxGeometryElementUV* vertexUV = pMesh->GetElementUV(inUVLayer);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBX::ReadNormal(FbxMesh* pMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);

	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

//void FBX::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
//{
//	if (inMesh->GetElementBinormalCount() < 1)
//	{
//		throw std::exception("Invalid Binormal Number");
//	}
//
//	FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
//	switch (vertexBinormal->GetMappingMode())
//	{
//	case FbxGeometryElement::eByControlPoint:
//		switch (vertexBinormal->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//		{
//			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
//			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
//			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
//		}
//		break;
//
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
//			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
//			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
//			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
//		}
//		break;
//
//		default:
//			throw std::exception("Invalid Reference");
//		}
//		break;
//
//	case FbxGeometryElement::eByPolygonVertex:
//		switch (vertexBinormal->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//		{
//			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
//			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
//			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
//		}
//		break;
//
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
//			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
//			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
//			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
//		}
//		break;
//
//		default:
//			throw std::exception("Invalid Reference");
//		}
//		break;
//	}
//}
//
//void FBX::ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent)
//{
//	if (inMesh->GetElementTangentCount() < 1)
//	{
//		throw std::exception("Invalid Tangent Number");
//	}
//
//	FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
//	switch (vertexTangent->GetMappingMode())
//	{
//	case FbxGeometryElement::eByControlPoint:
//		switch (vertexTangent->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//		{
//			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
//			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
//			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
//		}
//		break;
//
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
//			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
//			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
//			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
//		}
//		break;
//
//		default:
//			throw std::exception("Invalid Reference");
//		}
//		break;
//
//	case FbxGeometryElement::eByPolygonVertex:
//		switch (vertexTangent->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//		{
//			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
//			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
//			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
//		}
//		break;
//
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
//			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
//			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
//			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
//		}
//		break;
//
//		default:
//			throw std::exception("Invalid Reference");
//		}
//		break;
//	}
//}

// �����￡ ���͸��� �ε����� �����ϴ� �Լ�
void FBX::AssociateMaterialToMesh(FbxMesh* pMesh)
{
	FbxLayerElementArrayTemplate<int>* materialIndices = nullptr;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;

	// ���͸��� ���� Ȯ��
	if (pMesh->GetElementMaterial())
	{
		materialIndices = &(pMesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();

		if (materialIndices)
		{
			switch (materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (materialIndices->GetCount() == m_TriangleCountVector->at(m_MeshCount))
				{
					for (unsigned int i = 0; i < m_TriangleCountVector->at(m_MeshCount); ++i)
					{
						unsigned int materialIndex = materialIndices->GetAt(i);

						// �����￡ ���͸��� �ε��� ����
						m_TrianglesVector->at(m_MeshCount)->m_Triangles[i].m_MaterialIndex = materialIndex;
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				unsigned int materialIndex = materialIndices->GetAt(0);
				for (unsigned int i = 0; i < m_TriangleCountVector->at(m_MeshCount); ++i)
				{
					// �����￡ ���͸��� �ε��� ����
					m_TrianglesVector->at(m_MeshCount)->m_Triangles[i].m_MaterialIndex = materialIndex;
				}
			}
			break;

			default:
			{
				MessageBox(m_hwnd, L"FBX.cpp : Invalid mapping mode for material", L"Error", MB_OK);
			}
			break;
			}
		}
	}
}

// ���͸����� �����ϴ� �Լ�
void FBX::ProcessMaterials(FbxNode* pNode)
{
	// ���͸��� ���� ����
	unsigned int materialCount = pNode->GetMaterialCount();

	// ���͸����� �ִ� ��쿡�� m_MaterialLookUpVector�� ����
	if (materialCount)
	{
		MaterialUnorderedMap* materialLookUp = new MaterialUnorderedMap;
		m_MaterialLookUpVector->push_back(materialLookUp);
	}

	// ���͸��� ���� ��ŭ ����
	for (unsigned int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = pNode->GetMaterial(i); // ǥ�� �� : <Phong> or <Lambert>
		ProcessMaterialAttribute(surfaceMaterial, i);
		ProcessMaterialTexture(surfaceMaterial, m_MaterialLookUpVector->at(m_MeshCount)->m_MaterialLookUp[i]);
	}
}

// ���͸��� ���� �����ϴ� �Լ�
void FBX::ProcessMaterialAttribute(FbxSurfaceMaterial* pMaterial, unsigned int inMaterialIndex)
{
	// m_MaterialLookUp�� ������ Material ���� �Ҵ�
	Material* currMaterial = new Material();

	FbxDouble3 double3;
	FbxDouble double1;

	if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId)) // Phong ���̶��
	{
		currMaterial->Phong = true;
		currMaterial->Lambert = false;

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Specular;
		currMaterial->mSpecular.x = static_cast<float>(double3.mData[0]);
		currMaterial->mSpecular.y = static_cast<float>(double3.mData[1]);
		currMaterial->mSpecular.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Reflection;
		currMaterial->mReflection.x = static_cast<float>(double3.mData[0]);
		currMaterial->mReflection.y = static_cast<float>(double3.mData[1]);
		currMaterial->mReflection.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;

		// Shininess
		double1 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->Shininess;
		currMaterial->mShininess = double1;

		// Specular Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->SpecularFactor;
		currMaterial->mSpecularPower = double1;


		// Reflection Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(pMaterial)->ReflectionFactor;
		currMaterial->mReflectionFactor = double1;
	}
	else if (pMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId)) // Lambert ���̶��
	{
		currMaterial->Phong = false;
		currMaterial->Lambert = true;

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(pMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfaceLambert *>(pMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;
	}

	// ���� �Ҵ��� Material�� m_MaterialLookUp�� ����
	m_MaterialLookUpVector->at(m_MeshCount)->m_MaterialLookUp[inMaterialIndex] = currMaterial;
}

// ���͸��� �ؽ�ó�� �����ο� ����θ� �����ϴ� �Լ�
void FBX::ProcessMaterialTexture(FbxSurfaceMaterial* pFbxSurfaceMaterial, Material* pMaterial)
{
	unsigned int textureIndex = 0;
	FbxProperty property;

	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = pFbxSurfaceMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);

		if (property.IsValid())
		{
			unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
			for (unsigned int i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if (layeredTexture)
				{
					//throw std::exception("Layered Texture is currently unsupported\n");
				}
				else
				{
					FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
					if (texture)
					{
						std::string textureType = property.GetNameAsCStr();
						FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

						if (fileTexture) // �ؽ�ó Ÿ�� Ȯ��
						{
							if (textureType == "DiffuseColor")
							{
								pMaterial->mDiffuseMapName = fileTexture->GetFileName();
								pMaterial->mDiffuseMapRelativeName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "SpecularColor")
							{
								pMaterial->mSpecularMapName = fileTexture->GetFileName();
								pMaterial->mSpecularMapRelativeName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "Bump")
							{
								pMaterial->mNormalMapName = fileTexture->GetFileName();
								pMaterial->mNormalMapRelativeName = fileTexture->GetRelativeFileName();
							}
						}
					}
				}
			}
		}
	}
}

// ����Ʈ �����ϴ� �Լ��ε� ���� ������� ����
void FBX::FillLightNodeVectorRecursively(FbxNode* pNode)
{
	for (unsigned int i = 0; i < m_AnimationStackVector->size(); i++)
	{
		FbxAnimLayer* currentAnimLayer = m_AnimationStackVector->at(i)->m_AnimStack->GetMember<FbxAnimLayer>();

		FbxLight* light = pNode->GetLight();
		if (light)
		{
			LightCache* lightCache = new LightCache;;

			lightCache->m_Type = light->LightType.Get();

			FbxPropertyT<FbxDouble3> colorProperty = light->Color;
			FbxDouble3 lLightColor = colorProperty.Get();

			lightCache->m_ColorRed = static_cast<float>(lLightColor[0]);
			lightCache->m_ColorGreen = static_cast<float>(lLightColor[1]);
			lightCache->m_ColorBlue = static_cast<float>(lLightColor[2]);

			if (currentAnimLayer)
			{
				lightCache->m_ColorRedAnimCurve = colorProperty.GetCurve(currentAnimLayer, FBXSDK_CURVENODE_COLOR_RED);
				lightCache->m_ColorGreenAnimCurve = colorProperty.GetCurve(currentAnimLayer, FBXSDK_CURVENODE_COLOR_GREEN);
				lightCache->m_ColorBlueAnimCurve = colorProperty.GetCurve(currentAnimLayer, FBXSDK_CURVENODE_COLOR_BLUE);
			}

			if (lightCache->m_Type == FbxLight::eSpot)
			{
				FbxPropertyT<FbxDouble> coneAngleProperty = light->InnerAngle;
				lightCache->m_ConeAngle = static_cast<float>(coneAngleProperty.Get());
				if (currentAnimLayer)
					lightCache->m_ConeAngleAnimCurve = coneAngleProperty.GetCurve(currentAnimLayer);
			}

			if (lightCache->m_Type == FbxLight::eDirectional)
			{
				// �ϴ� ����
			}

			// ���Ϳ� �߰�
			m_LightCacheVector->push_back(lightCache);
		}

	}
}

void FBX::ProcessGlobalOffPosition(FbxNode* pNode)
{
	FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(NULL);
	FbxAMatrix lGeometryOffset = GetGeometry(pNode);
	FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

	m_GlobalOffPosition->push_back(ConvertFbxAMatrixtoXMMATRIX(lGlobalOffPosition));
}

void FBX::ProcessFinalTransform(FbxNode* pNode)
{
	AnimationVector animStack;

	for (auto iterAS = m_AnimationStackVector->begin(); iterAS != m_AnimationStackVector->end(); iterAS++)
	{
		FbxTime mFrameTime;
		mFrameTime.SetTime(0, 0, 0, 1, 0, m_FBXScene->GetGlobalSettings().GetTimeMode());

		// Default Pose ����
		FbxPose* defaultPose = nullptr;
		if (!m_PoseVector->empty())
			defaultPose = m_PoseVector->at(0);

		// Default AnimStack ����
		FbxAnimLayer* defaultAnimLayer = nullptr;
		defaultAnimLayer = (*iterAS)->m_AnimStack->GetMember<FbxAnimLayer>();
		m_FBXScene->SetCurrentAnimationStack((*iterAS)->m_AnimStack);

		FbxAMatrix pParentGlobalPosition; // ��������� �ʽ��ϴ�.

		Clusters clusters;

		for (auto iterC = m_Skeleton->m_Clusters.begin(); iterC != m_Skeleton->m_Clusters.end(); iterC++)
		{
			Cluster animationTransform;

			for (auto time = (*iterAS)->m_StartTime; time < (*iterAS)->m_EndTime; time += mFrameTime)
			{
				FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, time, defaultPose, &pParentGlobalPosition);
				FbxAMatrix lGeometryOffset = GetGeometry(pNode);
				FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;

				FbxAMatrix lVertexTransformMatrix;
				ComputeClusterDeformation(lGlobalOffPosition, pNode->GetMesh(), iterC->second, lVertexTransformMatrix, time, defaultPose);

				animationTransform.frameMatrix.push_back(ConvertFbxAMatrixtoXMMATRIX(lVertexTransformMatrix));
			}

			clusters.clustersUMAP[iterC->first] = animationTransform;
		}

		animStack.animVector.push_back(clusters);
	}

	m_FinalTransform->emplace(std::pair<unsigned int, AnimationVector>(m_MeshCount, animStack));
}

XMMATRIX FBX::ConvertFbxAMatrixtoXMMATRIX(FbxAMatrix fbxAMatrix)
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