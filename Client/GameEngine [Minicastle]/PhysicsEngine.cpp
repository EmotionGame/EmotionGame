#include "stdafx.h"
#include "HID.h"
#include "RenderingEngine.h"
#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine()
{
}

PhysicsEngine::PhysicsEngine(const PhysicsEngine& rOther)
{
}

PhysicsEngine::~PhysicsEngine()
{
}

bool PhysicsEngine::Initialize(HWND hwnd)
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

bool PhysicsEngine::Frame(RenderingEngine* pRenderingEngine, HID* pHID, NetworkEngine* pNetworkEngine, float deltaTime)
{
	pRenderingEngine->Physics(pHID, pNetworkEngine, deltaTime);

	return true;
}
