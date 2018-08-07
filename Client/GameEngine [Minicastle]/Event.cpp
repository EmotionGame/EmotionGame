#include "stdafx.h"
#include "Model.h"
#include "Event.h"

void Event::SetState(bool state)
{
	m_State = state;
}

bool Event::GetState()
{
	return m_State;
}