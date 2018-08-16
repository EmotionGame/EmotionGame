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
	XMFLOAT3 m_Axis[3] = { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }; // 순서 : right, up, lookAt
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

	// 교차 검사
	bool IntersectionAABB(const AABB* aabb1, const AABB* aabb2);
	bool IntersectionOBB(const OBB* obb1, const OBB* obb2);
	bool IntersectionAABB_OBB(const AABB* aabb, const OBB* obb);

	void SetCollisionCheck(bool collisionCheck);
	bool GetCollisionCehck();

private:
	// m_Vertex을 계산
	void CalculateAABBVertex();
	void CalculateOBBVertex();

	// 선 렌더링을 위해 다시 계산
	void ResetAABB(XMMATRIX worldMatrix);
	void ResetOBB(XMMATRIX worldMatrix, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 lookAt, XMFLOAT3 rotation);

public:
	ID3D11Device* m_Device; // 가져온 포인터이므로 nullptr를 대입 금지
	HWND m_hwnd;

	int m_CollisionType = AxisAlignedBoundingBox;
	bool m_CollisionCheck = false; // 충돌 판정 플래그

	Line m_Line;

	// Line에 넘겨주어 렌더링하기 위한 8개의 정점
	XMFLOAT3 m_Vertex[8];

	// 초기값을 저장
	AABB m_InitAABB;
	OBB m_InitOBB;

	// 실제값을 저장
	AABB m_AABB;
	OBB m_OBB;

	XMMATRIX m_WorldMatrixOBB = XMMatrixIdentity();

	bool m_FirstRenderCheck = false;
};