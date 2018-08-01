#pragma once

class HID;

class PhysicsEngine : public AlignedAllocationPolicy<16>
{
public:
	PhysicsEngine();
	PhysicsEngine(const PhysicsEngine& other);
	~PhysicsEngine();

	bool Initialize(HWND hwnd, HID* pHid);
	void Shutdown();
	bool Frame(float deltaTime);

private:
	HWND m_hwnd;
};