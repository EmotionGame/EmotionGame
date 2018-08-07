#include "stdafx.h"
#include "Model.h"
#include "Player.h"

void Player::SetHP(int hp)
{
	m_HP = hp;
}
void Player::SetSpeed(int speed)
{
	m_Speed = speed;

}
void Player::SetEmotion(int* emotion)
{
	for (int i = 0; i < 4; i++)
	{
		m_Emotion[i] = emotion[i];
	}
}

int Player::GetHP()
{


	return m_HP;
}
int Player::GetSpeed()
{


	return m_Speed;
}
int* Player::GetEmotion()
{


	return m_Emotion;
}