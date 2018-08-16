#pragma once

class Model;

class Object : public Model
{
public:
	void SetHP(int hp);
	void SetEmotion(int* emotion);

	int GetHP();
	int* GetEmotion();
	
private:
	int m_HP = 40;
	int m_Emotion[4] = { 0, 0, 0, 0 };

};