#pragma once

class HID;
class QuadTree;
class Model;

class Player : public Model
{
public:
	void SetHP(int hp);
	void SetSpeed(int speed);
	void SetEmotion(int* emotion);
	void SetAcceleration(float acceleration[3]);

	int GetHP();
	int GetSpeed();
	int* GetEmotion();
	XMFLOAT3 GetAcceleration();

	void PlayerAnimation(float deltaTime);

	void MoveObejctToLookAt();
	void MoveObejctToLookAtSide();
	void RotateObject(XMFLOAT3 value);
	void PlayerControl(HID* pHID, QuadTree* QuadTree, float frameTime);

private:
	int m_HP = 100;
	int m_Speed = 10;
	int m_Emotion[4] = { 0, 0, 0, 0 };

	XMFLOAT3 m_Acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);
};