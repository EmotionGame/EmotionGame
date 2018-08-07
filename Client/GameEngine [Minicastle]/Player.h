#pragma once

class Model;

class Player : public Model
{
public:
	void SetHP(int hp);
	void SetSpeed(int speed);
	void SetEmotion(int* emotion);

	int GetHP();
	int GetSpeed();
	int* GetEmotion();

private:
	int m_HP = 100;
	int m_Speed = 10;
	int m_Emotion[4] = { 0, 0, 0, 0 };
};