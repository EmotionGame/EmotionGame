#include "stdafx.h"
#include "Model.h"
#include "Monster.h"

void Monster::SetSpeed(int speed)
{
	m_Speed = speed;

}
void Monster::SetEmotion(int* emotion)
{
	for (int i = 0; i < 4; i++)
	{
		m_Emotion[i] = emotion[i];
	}
}
void Monster::SetDamage(int damage)
{
	m_Damage = damage;
}

int Monster::GetSpeed()
{
		return m_Speed;
}
int* Monster::GetEmotion()
{
		return m_Emotion;
}
int Monster::GetDamage()
{
	return m_Damage;
}