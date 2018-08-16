#pragma once

class EmotionShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct EmotionBufferType
	{
		int maxEmotionValue;
		int maxEmotionKind;
		int screenWidth;
		int screenHeight;
	};

public:
	EmotionShader();
	EmotionShader(const EmotionShader& rOther);
	~EmotionShader();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix,XMMATRIX viewMatrix, XMMATRIX projectionMatrix, 
		ID3D11ShaderResourceView* pTexture, int maxEmotionValue, int maxEmotionKind, int screenWidth, int screenHeight);

private:
	bool InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFilename, WCHAR* pPixelShaderFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName);

	bool SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* pTexture, int maxEmotionValue, int maxEmotionKind, int screenWidth, int screenHeight);
	void RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount);

private:
	HWND m_hwnd;

	ID3D11VertexShader * m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11Buffer* m_emotionBuffer = nullptr;

	// 여기에는 샘플러 상태의 포인터에 해당하는 새로운 private 변수가 있습니다.
	// 이 포인터는 텍스쳐 셰이더와의 인터페이스로서 사용될 것입니다
	// 필터링에 사용되는 것
	ID3D11SamplerState* m_sampleState = nullptr;
};