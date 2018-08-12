#pragma once

class ColorShader
{
private:
	struct CollisionCheckBufferType
	{
		bool collisionCheck;
		bool padding[15];
	};

public:
	ColorShader();
	//ColorShader(const ColorShader& rOther);
	~ColorShader();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck);
private:
	bool InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName);

	bool SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck);
	void RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount);
private:
	HWND m_hwnd;

	ID3D11VertexShader * m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_layout;

	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_collisionCheckBuffer;
};