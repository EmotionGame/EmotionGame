#pragma once

class LocalLightShader
{
public:
	LocalLightShader();
	LocalLightShader(const LocalLightShader& other);
	~LocalLightShader();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* pTexture, XMFLOAT3 lightDirection, XMFLOAT3 cameraPosition, XMFLOAT4 ambientColor[], XMFLOAT4 diffuseColor[], XMFLOAT4 specularPower[], XMFLOAT4 specularColor[]);

private:
	bool InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT3, XMFLOAT4[], XMFLOAT4[], XMFLOAT4[], XMFLOAT4[]);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	HWND m_hwnd;

	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;

	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11Buffer* m_cameraBuffer = nullptr;
	ID3D11Buffer* m_lightBuffer = nullptr;
};