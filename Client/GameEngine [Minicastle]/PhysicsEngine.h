#pragma once

class HID;
class RenderingEngine;

class PhysicsEngine : public AlignedAllocationPolicy<16>
{
public:
	PhysicsEngine();
	PhysicsEngine(const PhysicsEngine& rOther);
	~PhysicsEngine();

	bool Initialize(HWND hwnd);
	void Shutdown();
	bool Frame(RenderingEngine* pRenderingEngine, HID* pHID, NetworkEngine* pNetworkEngine, float deltaTime);

private:
	HWND m_hwnd;
};