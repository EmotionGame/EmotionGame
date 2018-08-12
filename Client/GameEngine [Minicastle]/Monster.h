#pragma once

class Model;

class Monster : public Model
{
public:
	void SetSpeed(int speed);
	void SetEmotion(int* emotion);
	void SetDamage(int damage);

	int GetSpeed();
	int* GetEmotion();
	int GetDamage();

private:
	int m_Speed = 10;
	int m_Emotion[4] = { 0, 0, 0, 0 };
	int m_Damage = 20;
};