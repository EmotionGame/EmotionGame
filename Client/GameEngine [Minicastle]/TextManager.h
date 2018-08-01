#pragma once

class Text;

class TextManager : public AlignedAllocationPolicy<16>
{
public:
	TextManager();
	TextManager(const TextManager& other);
	~TextManager();

	bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, HWND hwnd, int screenWidth, int screenHeight, XMMATRIX baseViewMatrix);
	void Shutdown();
	bool Frame(ID3D11DeviceContext* pDeviceContext, int cputPercentage, int fps, float deltaTime, float averageDeltaTime, int playerID, int TerrainPolygonCount);
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix);

private:
	HWND m_hwnd;
	
	std::vector<Text*>* m_Texts = new std::vector<Text*>;
};