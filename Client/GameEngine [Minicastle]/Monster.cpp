#include "stdafx.h"
#include "Model.h"
#include "Monster.h"

void Monster::PlayerAnimation(float deltaTime)
{
	/***** 애니메이션 재생 관리 : 시작 *****/
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
	/***** 애니메이션 재생 관리 : 종료 *****/
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