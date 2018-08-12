#pragma once

#include "ColorShader.h"

class Line
{
private:
	struct LineVertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	Line();
	Line(const Line& rOther);
	Line& operator=(const Line& rOther);
	~Line();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, XMFLOAT3 vertex[8], XMFLOAT4 color);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck);

	bool InitializeBuffers(ID3D11Device* pDevice, XMFLOAT3 vertex[8], XMFLOAT4 color);

private:
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* pDeviceContext);

private:
	HWND m_hwnd;

	ID3D11Buffer* m_VertexBuffer;
	ID3D11Buffer* m_IndexBuffer;

	ColorShader m_ColorShader;
};