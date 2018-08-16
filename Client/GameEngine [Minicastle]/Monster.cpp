#include "stdafx.h"
#include "Model.h"
#include "Monster.h"

void Monster::PlayerAnimation(float deltaTime)
{
	/***** �ִϸ��̼� ��� ���� : ���� *****/
	m_SumDeltaTime += deltaTime;
	if (m_SumDeltaTime > 33.33f)
	{
		m_AnimFrameCount++;
		if (m_AnimFrameCount >= m_AnimFrameSize)
		{
			m_AnimFrameCount = 0;
		}
		m_SumDeltaTime = 0.0f;
	}
	/***** �ִϸ��̼� ��� ���� : ���� *****/
}

void Monster::SetSpeed(float speed)
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

float Monster::GetSpeed()
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