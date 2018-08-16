#include "stdafx.h"
#include "HID.h"
#include "QuadTree.h"
#include "Model.h"
#include "Player.h"

void Player::PlayerAnimation(float deltaTime)
{
	/***** 애니메이션 재생 관리 : 시작 *****/
	m_SumDeltaTime += deltaTime * m_Acceleration.z * 75.0f;
	if (m_SumDeltaTime > 41.66f) // 1초당 24프레임
	{
		m_AnimFrameCount++;
		if (m_AnimFrameCount >= m_AnimFrameSize)
		{
			m_AnimFrameCount = 0;
		}
		m_SumDeltaTime = 0.0f;
	}
	if (m_SumDeltaTime < -41.66f) // 1초당 24프레임
	{
		if (m_AnimFrameCount == 0)
		{
			m_AnimFrameCount = m_AnimFrameSize - 1;
		}
		else
		{
			m_AnimFrameCount--;
		}
		m_SumDeltaTime = 0.0f;
	}
	/***** 애니메이션 재생 관리 : 종료 *****/
}

void Player::MoveObejctToLookAt(float deltaTime)
{
	m_ModelTranslation.x += m_Acceleration.z * m_Speed * m_LootAt.x * deltaTime * 0.07f;
	//m_ModelTranslation.y += value.y * m_LootAt.y;
	m_ModelTranslation.z += m_Acceleration.z * m_Speed * m_LootAt.z * deltaTime * 0.07f;

	if (m_ModelTranslation.x < 20.0f)
		m_ModelTranslation.x = 20.0f;
	if (m_ModelTranslation.x > 230.0f)
		m_ModelTranslation.x = 230.0f;
}
void Player::MoveObejctToLookAtSide(float deltaTime)
{
	m_ModelTranslation.x += m_Acceleration.x * m_Speed * m_Side.x * deltaTime * 0.05f;
	//m_ModelTranslation.y += value.y * m_Side.y;
	m_ModelTranslation.z += m_Acceleration.x * m_Speed * m_Side.z * deltaTime * 0.05f;

	if (m_ModelTranslation.z < 22.0f)
		m_ModelTranslation.z = 22.0f;
	if (m_ModelTranslation.z > 237.0f)
		m_ModelTranslation.z = 237.0f;
}
void Player::RotateObject(XMFLOAT3 value)
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
void Player::PlayerControl(HID* pHID, QuadTree* pQuadTree, float deltaTime)
{
	/***** 시점 : 시작 *****/
	int mouseX, mouseY;
	pHID->GetMouse_Keyboard()->GetDeltaMouse(mouseX, mouseY);

	float rotateSpeed;

	if (mouseX > 100)
		mouseX = 0;
	if (mouseY > 100)
		mouseY = 0;

	if (deltaTime > 1000.0f)
	{
		rotateSpeed = 0.05f;
	}
	else
	{
		rotateSpeed = 0.05f * deltaTime;
	}

	RotateObject(XMFLOAT3(rotateSpeed * mouseY, rotateSpeed * mouseX, 0.0f));
	/***** 시점 : 종료 *****/

	/***** 이동 : 시작 *****/
	if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_W))
	{
		if (m_Acceleration.z < 0.03f)
		{
			m_Acceleration.z += 0.0002f * deltaTime;
		}
	}
	if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_S))
	{
		if (m_Acceleration.z > -0.03f)
		{
			m_Acceleration.z -= 0.0002f * deltaTime;
		}
	}
	if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_A))
	{
		if (m_Acceleration.x > -0.015f)
		{
			m_Acceleration.x -= 0.0001f * deltaTime;
		}
	}
	if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_D))
	{
		if (m_Acceleration.x < 0.015f)
		{
			m_Acceleration.x += 0.0001f * deltaTime;
		}
	}

	MoveObejctToLookAt(deltaTime);
	MoveObejctToLookAtSide(deltaTime);

	if (m_Acceleration.z > 0.0f)
	{
		m_Acceleration.z -= 0.00015f * deltaTime;
		if (m_Acceleration.z < 0.0f)
		{
			m_Acceleration.z = 0.0f;
		}
	}
	if (m_Acceleration.z < 0.0f)
	{
		m_Acceleration.z += 0.00015f * deltaTime;
		if (m_Acceleration.z > 0.0f)
		{
			m_Acceleration.z = 0.0f;
		}
	}
	if (m_Acceleration.x > 0.0f)
	{
		m_Acceleration.x -= 0.000075f * deltaTime;
		if (m_Acceleration.x < 0.0f)
		{
			m_Acceleration.x = 0.0f;
		}
	}
	if (m_Acceleration.x < 0.0f)
	{
		m_Acceleration.x += 0.000075f * deltaTime;
		if (m_Acceleration.x > 0.0f)
		{
			m_Acceleration.x = 0.0f;
		}
	}
	/***** 이동 : 종료 *****/

	/***** 점프 : 시작 *****/
	float PosY;
	if (pQuadTree->GetHeightAtPosition(m_ModelTranslation.x, m_ModelTranslation.z, PosY))
	{
		if (pHID->GetMouse_Keyboard()->IsKeyDown(DIK_SPACE))
		{
			if (PosY == m_ModelTranslation.y)
			{
				m_Acceleration.y = 0.2f;
			}
		}
	}
 
	m_ModelTranslation.y += m_Acceleration.y * m_Up.y * deltaTime * 0.07f;

	m_Acceleration.y -= 0.0005f * deltaTime;

	if (pQuadTree->GetHeightAtPosition(m_ModelTranslation.x, m_ModelTranslation.z, PosY))
	{
		if (m_ModelTranslation.y < PosY)
		{
			m_ModelTranslation.y = PosY;
			m_Acceleration.y = 0.0f;
		}
	}
	/***** 점프 : 종료 *****/

	CalculateWorldMatrix();
	CalculateCameraPosition();
}

void Player::DiedControl(HID* pHID, QuadTree* pQuadTree, float deltaTime)
{
	/***** 시점 : 시작 *****/
	int mouseX, mouseY;
	pHID->GetMouse_Keyboard()->GetDeltaMouse(mouseX, mouseY);

	float rotateSpeed;

	if (mouseX > 100)
		mouseX = 0;
	if (mouseY > 100)
		mouseY = 0;

	if (deltaTime > 1000.0f)
	{
		rotateSpeed = 0.05f;
	}
	else
	{
		rotateSpeed = 0.05f * deltaTime;
	}

	RotateObject(XMFLOAT3(rotateSpeed * mouseY, rotateSpeed * mouseX, 0.0f));
	/***** 시점 : 종료 *****/
}

void Player::SetHP(int hp)
{
	m_HP = hp;
}
void Player::SetSpeed(float speed)
{
	m_Speed = speed;

}
void Player::SetEmotion0(int emotion)
{
	m_Emotion[0] = emotion;
}
void Player::SetEmotion1(int emotion)
{
	m_Emotion[1] = emotion;
}
void Player::SetEmotion2(int emotion)
{
	m_Emotion[2] = emotion;
}
void Player::SetEmotion3(int emotion)
{
	m_Emotion[3] = emotion;
}
void Player::SetAcceleration(float acceleration[3])
{
	m_Acceleration.x = acceleration[0];
	m_Acceleration.y = acceleration[1];
	m_Acceleration.z = acceleration[2];
}

int Player::GetHP()
{
	return m_HP;
}
float Player::GetSpeed()
{
	return m_Speed;
}
int Player::GetEmotion0()
{
	return m_Emotion[0];
}
int Player::GetEmotion1()
{
	return m_Emotion[1];
}
int Player::GetEmotion2()
{
	return m_Emotion[2];
}
int Player::GetEmotion3()
{
	return m_Emotion[3];
}
XMFLOAT3 Player::GetAcceleration()
{
	return m_Acceleration;
}