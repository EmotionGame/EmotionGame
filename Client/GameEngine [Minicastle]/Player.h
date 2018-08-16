#pragma once

class HID;
class QuadTree;
class Model;

class Player : public Model
{
public:
	void SetHP(int hp);
	void SetSpeed(float speed);
	void SetEmotion0(int emotion);
	void SetEmotion1(int emotion);
	void SetEmotion2(int emotion);
	void SetEmotion3(int emotion);

	void SetAcceleration(float acceleration[3]);

	int GetHP();
	float GetSpeed();
	int GetEmotion0();
	int GetEmotion1();
	int GetEmotion2();
	int GetEmotion3();

	XMFLOAT3 GetAcceleration();

	void PlayerAnimation(float deltaTime);

	void MoveObejctToLookAt(float deltaTime);
	void MoveObejctToLookAtSide(float deltaTime);
	void RotateObject(XMFLOAT3 value);
	void PlayerControl(HID* pHID, QuadTree* QuadTree, float deltaTime);

	void DiedControl(HID* pHID, QuadTree* QuadTree, float deltaTime);


private:
	int m_HP = 100;
	float m_Speed = 10.0f;
	int m_Emotion[4] = { 0, 0, 0, 0 };

	XMFLOAT3 m_Acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);
};