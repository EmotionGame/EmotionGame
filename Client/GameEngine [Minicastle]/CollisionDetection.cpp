#include "stdafx.h"
#include <DirectXCollision.h> // DirectX���� �⺻������ �����ϴ� �浹 �˻� �ý���. ������, ������� �ʰ� ���� �����غ��ҽ��ϴ�.
#include "CollisionDetection.h"

CollisionDetection::CollisionDetection()
{
}
CollisionDetection::CollisionDetection(const CollisionDetection& rOther)
{
	m_Device = rOther.m_Device;
	m_hwnd = rOther.m_hwnd;

	m_CollisionType = rOther.m_CollisionType;
	m_CollisionCheck = rOther.m_CollisionCheck;

	m_Line = rOther.m_Line;

	for (int i = 0; i < 8; i++)
	{
		m_Vertex[i] = rOther.m_Vertex[i];
	}

	m_InitAABB = rOther.m_InitAABB;
	m_InitOBB = rOther.m_InitOBB;

	m_AABB = rOther.m_AABB;
	m_OBB = rOther.m_OBB;
}
CollisionDetection::~CollisionDetection()
{

}

bool CollisionDetection::Initilize(ID3D11Device* pDevice, HWND hwnd, int collisionType)
{
	m_Device = pDevice;
	m_hwnd = hwnd;
	m_CollisionType = collisionType;

	m_AABB = m_InitAABB;
	m_OBB = m_InitOBB;

	// �ٿ�� �ڽ� Ÿ�� Ȯ��
	switch (m_CollisionType)
	{
	case AxisAlignedBoundingBox:
		CalculateAABBVertex();
		m_Line.Initialize(pDevice, hwnd, m_Vertex, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
		break;

	case OrientedBoundingBox:
		CalculateOBBVertex();
		m_Line.Initialize(pDevice, hwnd, m_Vertex, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
		break;
	}

	return true;
}

void CollisionDetection::Shutdown()
{
	m_Line.Shutdown();
}

bool CollisionDetection::Frame()
{

	return true;
}

bool CollisionDetection::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 lookAt, XMFLOAT3 rotation, bool lineRenderFlag)
{
	// �ٿ�� �ڽ� Ÿ�� Ȯ��
	switch (m_CollisionType)
	{
	case AxisAlignedBoundingBox:
		ResetAABB(worldMatrix);
		CalculateAABBVertex();

		if (lineRenderFlag)
		{
			m_Line.InitializeBuffers(m_Device, m_Vertex, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
			m_Line.Render(pDeviceContext, XMMatrixIdentity(), viewMatrix, projectionMatrix, m_CollisionCheck);
		}
		break;

	case OrientedBoundingBox:
		ResetOBB(worldMatrix, right, up, lookAt, rotation);
		CalculateOBBVertex();

		if (lineRenderFlag)
		{
			m_Line.InitializeBuffers(m_Device, m_Vertex, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
			m_Line.Render(pDeviceContext, m_WorldMatrixOBB, viewMatrix, projectionMatrix, m_CollisionCheck);
		}
		break;
	}

	m_FirstRenderCheck = true;

	return true;
}

void CollisionDetection::CalculateAABBVertex()
{
	m_Vertex[0] = XMFLOAT3(m_AABB.m_Min.x, m_AABB.m_Max.y, m_AABB.m_Min.z);
	m_Vertex[1] = XMFLOAT3(m_AABB.m_Max.x, m_AABB.m_Max.y, m_AABB.m_Min.z);
	m_Vertex[2] = m_AABB.m_Max;
	m_Vertex[3] = XMFLOAT3(m_AABB.m_Min.x, m_AABB.m_Max.y, m_AABB.m_Max.z);
	m_Vertex[4] = m_AABB.m_Min;
	m_Vertex[5] = XMFLOAT3(m_AABB.m_Max.x, m_AABB.m_Min.y, m_AABB.m_Min.z);
	m_Vertex[6] = XMFLOAT3(m_AABB.m_Max.x, m_AABB.m_Min.y, m_AABB.m_Max.z);
	m_Vertex[7] = XMFLOAT3(m_AABB.m_Min.x, m_AABB.m_Min.y, m_AABB.m_Max.z);
}

void CollisionDetection::CalculateOBBVertex()
{
	m_Vertex[0] = XMFLOAT3(m_OBB.m_Center.x - m_OBB.m_Extent.x, m_OBB.m_Center.y + m_OBB.m_Extent.y, m_OBB.m_Center.z - m_OBB.m_Extent.z);
	m_Vertex[1] = XMFLOAT3(m_OBB.m_Center.x + m_OBB.m_Extent.x, m_OBB.m_Center.y + m_OBB.m_Extent.y, m_OBB.m_Center.z - m_OBB.m_Extent.z);
	m_Vertex[2] = XMFLOAT3(m_OBB.m_Center.x + m_OBB.m_Extent.x, m_OBB.m_Center.y + m_OBB.m_Extent.y, m_OBB.m_Center.z + m_OBB.m_Extent.z);
	m_Vertex[3] = XMFLOAT3(m_OBB.m_Center.x - m_OBB.m_Extent.x, m_OBB.m_Center.y + m_OBB.m_Extent.y, m_OBB.m_Center.z + m_OBB.m_Extent.z);
	m_Vertex[4] = XMFLOAT3(m_OBB.m_Center.x - m_OBB.m_Extent.x, m_OBB.m_Center.y - m_OBB.m_Extent.y, m_OBB.m_Center.z - m_OBB.m_Extent.z);
	m_Vertex[5] = XMFLOAT3(m_OBB.m_Center.x + m_OBB.m_Extent.x, m_OBB.m_Center.y - m_OBB.m_Extent.y, m_OBB.m_Center.z - m_OBB.m_Extent.z);
	m_Vertex[6] = XMFLOAT3(m_OBB.m_Center.x + m_OBB.m_Extent.x, m_OBB.m_Center.y - m_OBB.m_Extent.y, m_OBB.m_Center.z + m_OBB.m_Extent.z);
	m_Vertex[7] = XMFLOAT3(m_OBB.m_Center.x - m_OBB.m_Extent.x, m_OBB.m_Center.y - m_OBB.m_Extent.y, m_OBB.m_Center.z + m_OBB.m_Extent.z);
}

bool CollisionDetection::IntersectionAABB(const AABB* aabb1, const AABB* aabb2) 
{
	if (!m_FirstRenderCheck)
		return false;

	return 
		(aabb1->m_Min.x <= aabb2->m_Max.x && aabb1->m_Max.x >= aabb2->m_Min.x) &&
		(aabb1->m_Min.y <= aabb2->m_Max.y && aabb1->m_Max.y >= aabb2->m_Min.y) &&
		(aabb1->m_Min.z <= aabb2->m_Max.z && aabb1->m_Max.z >= aabb2->m_Min.z);
}

bool CollisionDetection::IntersectionOBB(const OBB* obb1, const OBB* obb2) {
	if (!m_FirstRenderCheck)
		return false;

	// ���͸� �и��࿡ �����ϸ� ������ ���� ��ȯ
	// a���͸� b���ͷ� ����(�翵)�� ���� : abs(Dot(a, b);

	XMVECTOR obb1_Center = XMLoadFloat3(&obb1->m_Center);
	XMVECTOR obb1_Axis[3] = { XMLoadFloat3(&obb1->m_Axis[0]), XMLoadFloat3(&obb1->m_Axis[1]), XMLoadFloat3(&obb1->m_Axis[2]) };
	XMVECTOR obb1_Extent = XMLoadFloat3(&obb1->m_Extent);

	XMVECTOR obb2_Center = XMLoadFloat3(&obb2->m_Center);
	XMVECTOR obb2_Axis[3] = { XMLoadFloat3(&obb2->m_Axis[0]), XMLoadFloat3(&obb2->m_Axis[1]), XMLoadFloat3(&obb2->m_Axis[2]) };
	XMVECTOR obb2_Extent = XMLoadFloat3(&obb2->m_Extent);

	// obb1-2�� center ���� ����
	XMVECTOR D = obb1_Center - obb2_Center;
	
	float C[3][3];		// obb1-2�� axis�� �����Ͽ� ���� ���
	float absC[3][3];	// C�� ���밪 ���
	float AD[3];		// Dot(A_i, D)
	float R0;			// obb1�� interval radius
	float R1;			// obb2�� interval raduis
	float R;			// abs(Dot(A_i, D)) : obb1-2�� center�� ������ �Ÿ�
	float R01;			// R0 + R1

						// D3DXVec3Dot : �� ������ ������ ��ȯ�ϴ� �Լ�
						// fabsf : float�� ���밪 ��ȯ

	/***** A.x ������ ��, ���� : obb1->axis[0] *****/
	C[0][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[0]));
	C[0][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[1]));
	C[0][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[2]));
	AD[0] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], D));
	absC[0][0] = fabsf(C[0][0]);
	absC[0][1] = fabsf(C[0][1]);
	absC[0][2] = fabsf(C[0][2]);
	R = fabsf(AD[0]); // obb1�� x�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[0][0] + XMVectorGetY(obb2_Extent) * absC[0][1] + XMVectorGetZ(obb2_Extent) * absC[0][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** A.y ������ ��, ���� : obb1->axis[1] *****/
	C[1][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[0]));
	C[1][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[1]));
	C[1][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[2]));
	AD[1] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], D));
	absC[1][0] = fabsf(C[1][0]);
	absC[1][1] = fabsf(C[1][1]);
	absC[1][2] = fabsf(C[1][2]);
	R = fabsf(AD[1]); // obb1�� y�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetY(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[1][0] + XMVectorGetY(obb2_Extent) * absC[1][1] + XMVectorGetZ(obb2_Extent) * absC[1][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** A.z ������ ��, ���� : obb1->axis[2] *****/
	C[2][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[0]));
	C[2][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[1]));
	C[2][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[2]));
	AD[2] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], D));
	absC[2][0] = fabsf(C[2][0]);
	absC[2][1] = fabsf(C[2][1]);
	absC[2][2] = fabsf(C[2][2]);
	R = fabsf(AD[2]); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetZ(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[2][0] + XMVectorGetY(obb2_Extent) * absC[2][1] + XMVectorGetZ(obb2_Extent) * absC[2][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.x ������ ��, ���� : obb2->axis[0] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[0], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][0] + XMVectorGetY(obb1_Extent) * absC[1][0] + XMVectorGetZ(obb1_Extent) * absC[2][0];
	R1 = XMVectorGetX(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.y ������ ��, ���� : obb2->axis[1] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[1], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][1] + XMVectorGetY(obb1_Extent) * absC[1][1] + XMVectorGetZ(obb1_Extent) * absC[2][1];
	R1 = XMVectorGetY(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.z ������ ��, ���� : obb2->axis[2] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[2], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][2] + XMVectorGetY(obb1_Extent) * absC[1][2] + XMVectorGetZ(obb1_Extent) * absC[2][2];
	R1 = XMVectorGetZ(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.x) ������ �� *****/
	R = fabsf(C[1][0] * AD[2] - C[2][0] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][0] + XMVectorGetZ(obb1_Extent) * absC[1][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[0][2] + XMVectorGetZ(obb2_Extent) * absC[0][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.y) ������ �� *****/
	R = fabsf(C[1][1] * AD[2] - C[2][1] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][1] + XMVectorGetZ(obb1_Extent) * absC[1][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[0][2] + XMVectorGetZ(obb2_Extent) * absC[0][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.z) ������ �� *****/
	R = fabsf(C[1][2] * AD[2] - C[2][2] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][2] + XMVectorGetZ(obb1_Extent) * absC[1][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[0][1] + XMVectorGetY(obb2_Extent) * absC[0][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.x) ������ �� *****/
	R = fabsf(C[2][0] * AD[0] - C[0][0] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][0] + XMVectorGetZ(obb1_Extent) * absC[0][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[1][2] + XMVectorGetZ(obb2_Extent) * absC[1][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.y) ������ �� *****/
	R = fabsf(C[2][1] * AD[0] - C[0][1] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][1] + XMVectorGetZ(obb1_Extent) * absC[0][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[1][2] + XMVectorGetZ(obb2_Extent) * absC[1][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.z) ������ �� *****/
	R = fabsf(C[2][2] * AD[0] - C[0][2] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][2] + XMVectorGetZ(obb1_Extent) * absC[0][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[1][1] + XMVectorGetY(obb2_Extent) * absC[1][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.x) ������ �� *****/
	R = fabsf(C[0][0] * AD[1] - C[1][0] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][0] + XMVectorGetY(obb1_Extent) * absC[0][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[2][2] + XMVectorGetZ(obb2_Extent) * absC[2][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.y) ������ �� *****/
	R = fabsf(C[0][1] * AD[1] - C[1][1] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][1] + XMVectorGetY(obb1_Extent) * absC[0][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[2][2] + XMVectorGetZ(obb2_Extent) * absC[2][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.z) ������ �� *****/
	R = fabsf(C[0][2] * AD[1] - C[1][2] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][2] + XMVectorGetY(obb1_Extent) * absC[0][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[2][1] + XMVectorGetY(obb2_Extent) * absC[2][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	// 15���� �˻翡�� ��� ��ġ�� �� ��ü�� �浹
	return true;
}
bool CollisionDetection::IntersectionAABB_OBB(const AABB* aabb, const OBB* obb2) {
	if (!m_FirstRenderCheck)
		return false;

	// ���͸� �и��࿡ �����ϸ� ������ ���� ��ȯ
	// a���͸� b���ͷ� ����(�翵)�� ���� : abs(Dot(a, b);
	
	/***** AABB -> OBB ��ȯ : ���� *****/
	XMFLOAT3 right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 lookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMVECTOR obb1_Center = XMLoadFloat3(&
		XMFLOAT3((aabb->m_Min.x + aabb->m_Max.x) / 2.0f, (aabb->m_Min.y + aabb->m_Max.y) / 2.0f, (aabb->m_Min.z + aabb->m_Max.z) / 2.0f));
	XMVECTOR obb1_Axis[3] = { XMLoadFloat3(&right), XMLoadFloat3(&up), XMLoadFloat3(&lookAt) };
	XMVECTOR obb1_Extent = XMLoadFloat3(&
		XMFLOAT3((aabb->m_Max.x - aabb->m_Min.x) / 2.0f, (aabb->m_Max.y - aabb->m_Min.y) / 2.0f, (aabb->m_Max.z - aabb->m_Min.z) / 2.0f));
	/***** AABB -> OBB ��ȯ : ���� *****/

	XMVECTOR obb2_Center = XMLoadFloat3(&obb2->m_Center);
	XMVECTOR obb2_Axis[3] = { XMLoadFloat3(&obb2->m_Axis[0]), XMLoadFloat3(&obb2->m_Axis[1]), XMLoadFloat3(&obb2->m_Axis[2]) };
	XMVECTOR obb2_Extent = XMLoadFloat3(&obb2->m_Extent);

	// obb1-2�� center ���� ����
	XMVECTOR D = obb1_Center - obb2_Center;

	float C[3][3];		// obb1-2�� axis�� �����Ͽ� ���� ���
	float absC[3][3];	// C�� ���밪 ���
	float AD[3];		// Dot(A_i, D)
	float R0;			// obb1�� interval radius
	float R1;			// obb2�� interval raduis
	float R;			// abs(Dot(A_i, D)) : obb1-2�� center�� ������ �Ÿ�
	float R01;			// R0 + R1

						// D3DXVec3Dot : �� ������ ������ ��ȯ�ϴ� �Լ�
						// fabsf : float�� ���밪 ��ȯ

						/***** A.x ������ ��, ���� : obb1->axis[0] *****/
	C[0][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[0]));
	C[0][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[1]));
	C[0][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], obb2_Axis[2]));
	AD[0] = XMVectorGetX(XMVector3Dot(obb1_Axis[0], D));
	absC[0][0] = fabsf(C[0][0]);
	absC[0][1] = fabsf(C[0][1]);
	absC[0][2] = fabsf(C[0][2]);
	R = fabsf(AD[0]); // obb1�� x�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[0][0] + XMVectorGetY(obb2_Extent) * absC[0][1] + XMVectorGetZ(obb2_Extent) * absC[0][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** A.y ������ ��, ���� : obb1->axis[1] *****/
	C[1][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[0]));
	C[1][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[1]));
	C[1][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], obb2_Axis[2]));
	AD[1] = XMVectorGetX(XMVector3Dot(obb1_Axis[1], D));
	absC[1][0] = fabsf(C[1][0]);
	absC[1][1] = fabsf(C[1][1]);
	absC[1][2] = fabsf(C[1][2]);
	R = fabsf(AD[1]); // obb1�� y�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetY(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[1][0] + XMVectorGetY(obb2_Extent) * absC[1][1] + XMVectorGetZ(obb2_Extent) * absC[1][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** A.z ������ ��, ���� : obb1->axis[2] *****/
	C[2][0] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[0]));
	C[2][1] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[1]));
	C[2][2] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], obb2_Axis[2]));
	AD[2] = XMVectorGetX(XMVector3Dot(obb1_Axis[2], D));
	absC[2][0] = fabsf(C[2][0]);
	absC[2][1] = fabsf(C[2][1]);
	absC[2][2] = fabsf(C[2][2]);
	R = fabsf(AD[2]); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetZ(obb1_Extent);
	R1 = XMVectorGetX(obb2_Extent) * absC[2][0] + XMVectorGetY(obb2_Extent) * absC[2][1] + XMVectorGetZ(obb2_Extent) * absC[2][2];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.x ������ ��, ���� : obb2->axis[0] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[0], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][0] + XMVectorGetY(obb1_Extent) * absC[1][0] + XMVectorGetZ(obb1_Extent) * absC[2][0];
	R1 = XMVectorGetX(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.y ������ ��, ���� : obb2->axis[1] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[1], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][1] + XMVectorGetY(obb1_Extent) * absC[1][1] + XMVectorGetZ(obb1_Extent) * absC[2][1];
	R1 = XMVectorGetY(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** B.z ������ ��, ���� : obb2->axis[2] *****/
	R = fabsf(XMVectorGetX(XMVector3Dot(obb2_Axis[2], D))); // obb1�� z�࿡ ���Ͽ� obb1-2�� center�� ������ �Ÿ�
	R0 = XMVectorGetX(obb1_Extent) * absC[0][2] + XMVectorGetY(obb1_Extent) * absC[1][2] + XMVectorGetZ(obb1_Extent) * absC[2][2];
	R1 = XMVectorGetZ(obb2_Extent);
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.x) ������ �� *****/
	R = fabsf(C[1][0] * AD[2] - C[2][0] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][0] + XMVectorGetZ(obb1_Extent) * absC[1][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[0][2] + XMVectorGetZ(obb2_Extent) * absC[0][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.y) ������ �� *****/
	R = fabsf(C[1][1] * AD[2] - C[2][1] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][1] + XMVectorGetZ(obb1_Extent) * absC[1][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[0][2] + XMVectorGetZ(obb2_Extent) * absC[0][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.x, B.z) ������ �� *****/
	R = fabsf(C[1][2] * AD[2] - C[2][2] * AD[1]);
	R0 = XMVectorGetY(obb1_Extent) * absC[2][2] + XMVectorGetZ(obb1_Extent) * absC[1][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[0][1] + XMVectorGetY(obb2_Extent) * absC[0][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.x) ������ �� *****/
	R = fabsf(C[2][0] * AD[0] - C[0][0] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][0] + XMVectorGetZ(obb1_Extent) * absC[0][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[1][2] + XMVectorGetZ(obb2_Extent) * absC[1][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.y) ������ �� *****/
	R = fabsf(C[2][1] * AD[0] - C[0][1] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][1] + XMVectorGetZ(obb1_Extent) * absC[0][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[1][2] + XMVectorGetZ(obb2_Extent) * absC[1][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.y, B.z) ������ �� *****/
	R = fabsf(C[2][2] * AD[0] - C[0][2] * AD[2]);
	R0 = XMVectorGetX(obb1_Extent) * absC[2][2] + XMVectorGetZ(obb1_Extent) * absC[0][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[1][1] + XMVectorGetY(obb2_Extent) * absC[1][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.x) ������ �� *****/
	R = fabsf(C[0][0] * AD[1] - C[1][0] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][0] + XMVectorGetY(obb1_Extent) * absC[0][0];
	R1 = XMVectorGetY(obb2_Extent) * absC[2][2] + XMVectorGetZ(obb2_Extent) * absC[2][1];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.y) ������ �� *****/
	R = fabsf(C[0][1] * AD[1] - C[1][1] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][1] + XMVectorGetY(obb1_Extent) * absC[0][1];
	R1 = XMVectorGetX(obb2_Extent) * absC[2][2] + XMVectorGetZ(obb2_Extent) * absC[2][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	/***** cross(A.z, B.z) ������ �� *****/
	R = fabsf(C[0][2] * AD[1] - C[1][2] * AD[0]);
	R0 = XMVectorGetX(obb1_Extent) * absC[1][2] + XMVectorGetY(obb1_Extent) * absC[0][2];
	R1 = XMVectorGetX(obb2_Extent) * absC[2][1] + XMVectorGetY(obb2_Extent) * absC[2][0];
	R01 = R0 + R1;

	if (R > R01) // ��ġ�� ������
		return false;

	// 15���� �˻翡�� ��� ��ġ�� �� ��ü�� �浹
	return true;
}

void CollisionDetection::SetCollisionCheck(bool collisionCheck)
{
	m_CollisionCheck = collisionCheck;
}
bool CollisionDetection::GetCollisionCehck()
{
	return m_CollisionCheck;
}

void CollisionDetection::ResetAABB(XMMATRIX worldMatrix)
{
	/***** m_Min, m_Max ���� : ���� *****/
	XMVECTOR minVec = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(m_InitAABB.m_Min.x, m_InitAABB.m_Min.y, m_InitAABB.m_Min.z, 1.0f)), worldMatrix);
	XMFLOAT4 min;
	XMStoreFloat4(&min, minVec);

	XMVECTOR maxVec = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(m_InitAABB.m_Max.x, m_InitAABB.m_Max.y, m_InitAABB.m_Max.z, 1.0f)), worldMatrix);
	XMFLOAT4 max;
	XMStoreFloat4(&max, maxVec);

	m_AABB.m_Min.x = min.x <= max.x ? min.x : max.x;
	m_AABB.m_Min.y = min.y <= max.y ? min.y : max.y;
	m_AABB.m_Min.z = min.z <= max.z ? min.z : max.z;
	m_AABB.m_Max.x = min.x <= max.x ? max.x : min.x;
	m_AABB.m_Max.y = min.y <= max.y ? max.y : min.y;
	m_AABB.m_Max.z = min.z <= max.z ? max.z : min.z;
	/***** m_Min, m_Max ���� : ���� *****/
}

void CollisionDetection::ResetOBB(XMMATRIX worldMatrix, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 lookAt, XMFLOAT3 rotation)
{
	/***** m_Center ���� : ���� *****/
	XMVECTOR centerVec = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(m_InitOBB.m_Center.x, m_InitOBB.m_Center.y, m_InitOBB.m_Center.z, 1.0f)), worldMatrix);

	XMFLOAT4 center;
	XMStoreFloat4(&center, centerVec);

	m_OBB.m_Center = XMFLOAT3(center.x, center.y, center.z);
	/***** m_Center ���� : ���� *****/

	/***** m_Extent ���� : ���� *****/
	// ��Ʈ���� Decompose
	XMVECTOR outScale;
	XMVECTOR outRotQuat;
	XMVECTOR outTrans;
	XMMatrixDecompose(&outScale, &outRotQuat, &outTrans, worldMatrix);

	XMFLOAT3 scale;
	XMStoreFloat3(&scale, outScale);

	m_OBB.m_Extent.x = m_InitOBB.m_Extent.x * scale.x;
	m_OBB.m_Extent.y = m_InitOBB.m_Extent.y * scale.y;
	m_OBB.m_Extent.z = m_InitOBB.m_Extent.z * scale.z;
	/***** m_Extent ���� : ���� *****/

	/***** m_Axis[3] ���� : ���� *****/
	m_OBB.m_Axis[0] = right;
	m_OBB.m_Axis[1] = up;
	m_OBB.m_Axis[2] = lookAt;
	/***** m_Axis[3] ���� : ���� *****/

	/***** m_WorldMatrixOBB ���� : ���� *****/
	XMMATRIX tm_reverse = XMMatrixTranslation(-m_OBB.m_Center.x, -m_OBB.m_Center.y, -m_OBB.m_Center.z);
	XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(0.0f, rotation.y * XM_RADIAN, 0.0f);
	XMMATRIX rM = XMMatrixRotationQuaternion(vQ);
	XMMATRIX tm = XMMatrixTranslation(m_OBB.m_Center.x, m_OBB.m_Center.y, m_OBB.m_Center.z);

	m_WorldMatrixOBB = tm_reverse * rM * tm;
	/***** m_WorldMatrixOBB ���� : ���� *****/
}