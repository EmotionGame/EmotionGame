#pragma once

class Frustum : public AlignedAllocationPolicy<16>
{
public:
	Frustum();
	Frustum(const Frustum& rOther);
	~Frustum();

	void ConstructFrustum(float screenDepth, XMMATRIX projectionMatrix, XMMATRIX viewMatrix);

	bool CheckPoint(float x, float y, float z);
	bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);

private:
	XMVECTOR m_planes[6];
};