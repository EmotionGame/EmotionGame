#include "stdafx.h"
#include "HID.h"
#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine()
{
}

PhysicsEngine::PhysicsEngine(const PhysicsEngine& other)
{
}

PhysicsEngine::~PhysicsEngine()
{
}

bool PhysicsEngine::Initialize(HWND hwnd, HID* pHid)
{
#ifdef _DEBUG
	printf("Start >> PhysicsEngine.cpp : Initialize()\n");
#endif

	m_hwnd = hwnd;

#ifdef _DEBUG
	printf("Success >> PhysicsEngine.cpp : Initialize()\n");
#endif

	return true;
}

void PhysicsEngine::Shutdown()
{
	
}

bool PhysicsEngine::Frame(float deltaTime)
{


	return true;
}
