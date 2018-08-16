#include "stdafx.h"
#include "Model.h"
#include "Object.h"


void Object::SetHP(int hp)
{
	m_HP = hp;
}
void Object::SetEmotion(int* emotion)
{
	for (int i = 0; i < 4; i++)
	{
		m_Emotion[i] = emotion[i];
	}
}

int Object::GetHP()
{
	return m_HP;
}
int* Object::GetEmotion()
{
	return m_Emotion;
}