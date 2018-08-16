#pragma once

class Direct3D : public AlignedAllocationPolicy<16>
{
public:
	Direct3D();
	Direct3D(const Direct3D& other);
	~Direct3D();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	void Shutdown();

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX& projectionMatrix);
	void GetWorldMatrix(XMMATRIX& worldMatrix);
	void GetOrthoMatrix(XMMATRIX& orthoMatrix);

	void GetVideoCardInfo(char* pCardName, int& memory);

	void TurnZBufferOn();
	void TurnZBufferOff();
	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();
	void TurnOnCulling();
	void TurnOffCulling();

	/***** 감정 전달용 알파 스테이트 : 시작 *****/
	void TurnOnEmotionAlphaBlending();
	/***** 감정 전달용 알파 스테이트 : 종료 *****/
private:
	HWND m_hwnd;

	bool m_vsync_enabled = false;
	int m_videoCardMemory = 0;
	char m_videoCardDescription[128] = { 0, };
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11RasterizerState* m_rasterState = nullptr;
	ID3D11RasterizerState* m_rasterStateNoCulling = nullptr;
	XMMATRIX m_projectionMatrix = XMMatrixIdentity();
	XMMATRIX m_worldMatrix = XMMatrixIdentity();
	XMMATRIX m_orthoMatrix = XMMatrixIdentity();

	ID3D11DepthStencilState* m_depthDisabledStencilState = nullptr;
	ID3D11BlendState* m_alphaEnableBlendingState = nullptr;
	ID3D11BlendState* m_alphaDisableBlendingState = nullptr;

	/***** 감정 전달용 알파 스테이트 : 시작 *****/
	ID3D11BlendState* m_emotionAlphaState = nullptr;
	/***** 감정 전달용 알파 스테이트 : 종료 *****/
};