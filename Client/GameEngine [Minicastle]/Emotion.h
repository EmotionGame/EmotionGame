#pragma once

#include "Texture.h"
#include "EmotionShader.h"

class Direct3D;

class Emotion
{
private:
	struct EmotionVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};
public:
	Emotion();
	Emotion(const Emotion& rOther);
	~Emotion();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, int screenWidth, int screenHeight, WCHAR* pTextureFilename,
		int bitmapWidth, int bitmapHeight);
	void Shutdown();
	bool Render(Direct3D* pDirect3D, ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, int positionX, int positionY,
		int maxEmotionValue, int maxEmotionKind, int screenWidth, int screenHeight);
	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device* pDevice);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext* pDeviceContext, int positionX, int positionY);
	void RenderBuffers(ID3D11DeviceContext* pDeviceContext);
	void ReleaseTexture();

private:
	HWND m_hwnd;

	ID3D11Buffer * m_vertexBuffer = nullptr;
	ID3D11Buffer * m_indexBuffer = nullptr;
	int m_vertexCount, m_indexCount;

	Texture m_Texture;
	EmotionShader m_EmotionShader;

	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};