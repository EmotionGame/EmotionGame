#pragma once

class Model;

class Monster : public Model
{
public:
	void PlayerAnimation(float deltaTime);

	void SetSpeed(float speed);
	void SetEmotion(int* emotion);
	void SetDamage(int damage);

	float GetSpeed();
	int* GetEmotion();
	int GetDamage();

private:
	float m_Speed = 10.0f;
	int m_Emotion[4] = { 0, 0, 0, 0 };
	int m_Damage = 20;
};