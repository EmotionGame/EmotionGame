#pragma once

class HID;

class Camera : public AlignedAllocationPolicy<16>
{
public:
	Camera();
	Camera(const Camera& other);
	~Camera();

	bool Initialize();
	void Shutdown();
	bool Frame(HID* pHID, float frameTime);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xyz);
	void SetRotation(float x, float y, float z);
	void SetRotation(XMFLOAT3 xyz);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

	void RotateCamera(float x, float y, float z);
	void MoveCameraPositionToLookAt(float x, float y, float z);
	void MoveCameraPositionToLookAtUp(float x, float y, float z);
	void MoveCameraPositionToLookAtSide(float x, float y, float z);

	void Navigation(HID* pHID, float deltaTime);

	XMFLOAT3 GetLookAt();

private:
	bool m_Navigation = false;

	XMFLOAT3 m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX m_viewMatrix = XMMatrixIdentity();

	XMFLOAT3 m_lookAt;
	XMFLOAT3 m_up;
	XMFLOAT3 m_side;
};