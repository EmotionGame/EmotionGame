#pragma once

#include "Line.h"

struct AABB
{
	XMFLOAT3 m_Min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Max = XMFLOAT3(0.0f, 0.0f, 0.0f);
};
struct OBB
{
	XMFLOAT3 m_Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Axis[3] = { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }; // ���� : right, up, lookAt
	XMFLOAT3 m_Extent = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

class CollisionDetection : public AlignedAllocationPolicy<16>
{
public:
	CollisionDetection();
	CollisionDetection(const CollisionDetection& rOther);
	~CollisionDetection();

	bool Initilize(ID3D11Device* pDevice, HWND hwnd, int collisionType);
	void Shutdown();
	bool Frame();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
		XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 lookAt, XMFLOAT3 rotation, bool lineRenderFlag);

	// ���� �˻�
	bool IntersectionAABB(const AABB* aabb1, const AABB* aabb2);
	bool IntersectionOBB(const OBB* obb1, const OBB* obb2);
	bool IntersectionAABB_OBB(const AABB* aabb, const OBB* obb);

	void SetCollisionCheck(bool collisionCheck);
	bool GetCollisionCehck();

private:
	// m_Vertex�� ���
	void CalculateAABBVertex();
	void CalculateOBBVertex();

	// �� �������� ���� �ٽ� ���
	void ResetAABB(XMMATRIX worldMatrix);
	void ResetOBB(XMMATRIX worldMatrix, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 lookAt, XMFLOAT3 rotation);

public:
	ID3D11Device* m_Device; // ������ �������̹Ƿ� nullptr�� ���� ����
	HWND m_hwnd;

	int m_CollisionType = AxisAlignedBoundingBox;
	bool m_CollisionCheck = false; // �浹 ���� �÷���

	Line m_Line;

	// Line�� �Ѱ��־� �������ϱ� ���� 8���� ����
	XMFLOAT3 m_Vertex[8];

	// �ʱⰪ�� ����
	AABB m_InitAABB;
	OBB m_InitOBB;

	// �������� ����
	AABB m_AABB;
	OBB m_OBB;

	XMMATRIX m_WorldMatrixOBB = XMMatrixIdentity();

	bool m_FirstRenderCheck = false;
};