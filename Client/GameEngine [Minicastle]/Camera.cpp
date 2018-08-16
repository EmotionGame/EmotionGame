#include "stdafx.h"
#include "HID.h"
#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(const Camera& other)
{
}

Camera::~Camera()
{
}

bool Camera::Initialize()
{

	return true;
}

void Camera::Shutdown()
{
	// 포인터를 받아와서 사용하므로 m_HID->Shutdown() 금지
}

bool Camera::Frame(HID* pHID, float frameTime)
{
	Navigation(pHID, frameTime);

	return true;
}

void Camera::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}
void Camera::SetPosition(XMFLOAT3 xyz)
{
	m_position = xyz;
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}
void Camera::SetRotation(XMFLOAT3 xyz)
{
	m_rotation = xyz;
}

XMFLOAT3 Camera::GetPosition()
{
	return m_position;
}

XMFLOAT3 Camera::GetRotation()
{
	return m_rotation;
}

void Camera::Render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector, sideVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// 위쪽을 가리키는 벡터를 설정합니다.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// XMVECTOR 구조체에 로드한다.
	upVector = XMLoadFloat3(&up);

	// 3D월드에서 카메라의 위치를 ​​설정합니다.
	position = m_position;

	// XMVECTOR 구조체에 로드한다.
	positionVector = XMLoadFloat3(&position);

	// 기본적으로 카메라가 찾고있는 위치를 설정합니다.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// XMVECTOR 구조체에 로드한다.
	lookAtVector = XMLoadFloat3(&lookAt);

	// yaw (Y 축), pitch (X 축) 및 roll (Z 축)의 회전값을 라디안 단위로 설정합니다.
	pitch = m_rotation.x * XM_RADIAN;
	yaw = m_rotation.y * XM_RADIAN;
	roll = m_rotation.z * XM_RADIAN;

	// 쿼터니온으로 변환
	XMVECTOR rotationQuaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

	//  yaw, pitch, roll 값을 통해 회전 행렬을 만듭니다.
	rotationMatrix = XMMatrixRotationQuaternion(rotationQuaternion);

	// lookAt 및 up 벡터를 회전 행렬로 변형하여 뷰가 원점에서 올바르게 회전되도록 합니다.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// XMVECTOR -> XMFLOAT3
	XMStoreFloat3(&m_lookAt, lookAtVector);
	XMStoreFloat3(&m_up, upVector);
	sideVector = XMVector3Cross(lookAtVector, upVector);
	XMStoreFloat3(&m_side, sideVector);

	// 회전 된 카메라 위치를 뷰어 위치로 변환합니다.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// 마지막으로 세 개의 업데이트 된 벡터에서 뷰 행렬을 만듭니다.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void Camera::RotateCamera(float x, float y, float z) {
	// 끝없이 증가하거나 감소하는 것을 막기 위해 초기화
	if (m_rotation.x >= 360.0f)
		m_rotation.x += -360.0f;
	if (m_rotation.x <= -360.0f)
		m_rotation.x += 360.0f;

	if (m_rotation.y >= 360.0f)
		m_rotation.y += -360.0f;
	if (m_rotation.y <= -360.0f)
		m_rotation.y += 360.0f;

	if (m_rotation.z >= 360.0f)
		m_rotation.z += -360.0f;
	if (m_rotation.z <= -360.0f)
		m_rotation.z += 360.0f;

	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;
}

void Camera::MoveCameraPositionToLookAt(float x, float y, float z) 
{
	m_position.x += x * m_lookAt.x;
	m_position.y += y * m_lookAt.y;
	m_position.z += z * m_lookAt.z;
}
void Camera::MoveCameraPositionToLookAtUp(float x, float y, float z) 
{
	m_position.x += x * m_up.x;
	m_position.y += y * m_up.y;
	m_position.z += z * m_up.z;
}
void Camera::MoveCameraPositionToLookAtSide(float x, float y, float z) 
{
	m_position.x += x * m_side.x;
	m_position.y += y * m_side.y;
	m_position.z += z * m_side.z;
}

void Camera::Navigation(HID* pHID, float deltaTime) {
	if (pHID->GetMouse_Keyboard()->IsKeyRelease(DIK_F6))
	{
		if (m_Navigation)
			m_Navigation = false;
		else
			m_Navigation = true;
	}

	if (m_Navigation)
	{
		int mouseX, mouseY;
		pHID->GetMouse_Keyboard()->GetDeltaMouse(mouseX, mouseY);
		float moveSpeed = 0.05f * deltaTime;
		float rotateSpeed = 0.05f * deltaTime;

		RotateCamera(rotateSpeed * mouseY, rotateSpeed * mouseX, 0.0f);

		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_W))
			MoveCameraPositionToLookAt(moveSpeed, moveSpeed, moveSpeed);
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_S))
			MoveCameraPositionToLookAt(-moveSpeed, -moveSpeed, -moveSpeed);
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_E))
			MoveCameraPositionToLookAtUp(moveSpeed, moveSpeed, moveSpeed);
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_Q))
			MoveCameraPositionToLookAtUp(-moveSpeed, -moveSpeed, -moveSpeed);
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_A))
			MoveCameraPositionToLookAtSide(moveSpeed, moveSpeed, moveSpeed);
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_D))
			MoveCameraPositionToLookAtSide(-moveSpeed, -moveSpeed, -moveSpeed);
	}
}

XMFLOAT3 Camera::GetLookAt()
{
	return m_lookAt;
}