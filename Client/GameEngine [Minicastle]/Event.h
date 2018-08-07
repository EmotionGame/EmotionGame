#pragma once

class Model;

class Event : public Model
{
public:
	void SetState(bool state);

	bool GetState();

private:
	bool m_State = false;

};