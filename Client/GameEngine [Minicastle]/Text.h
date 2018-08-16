#pragma once

class Font;
class FontShader;

class Text : public AlignedAllocationPolicy<16>
{
private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct FontVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Text();
	Text(const Text&);
	~Text();

	bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, HWND hwnd, int screenWidth, int screenHeight, XMMATRIX baseViewMatrix,
		char* pFontdataTXT, wchar_t* pFontDDS, char* pSentences, int positionX, int positionY, float red, float green, float blue);

	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix);

	bool SetSentenceWithSTR(char* pFrontSentence, char* pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext);
	bool SetSentenceWithINT(char* pFrontSentence, int pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext);
	bool SetSentenceWithFLOAT(char* pFrontSentence, float pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext);
	bool SetSentenceWithINT4(char* pFrontSentence, int pData[4], char* pBackSentence, ID3D11DeviceContext* pDeviceContext);

private:
	bool InitializeSentence(SentenceType** ppSentence, int maxLength, ID3D11Device* pDevice);
	bool UpdateSentence(SentenceType* pSentence, char* text, ID3D11DeviceContext* pDeviceContext, int positionX, int positionY, float red, float green, float blue);
	void ReleaseSentence(SentenceType** ppSentence);
	bool RenderSentence(ID3D11DeviceContext* pDeviceContext, SentenceType* pSentence, XMMATRIX worldMatrix, XMMATRIX orthoMatrix);

private:
	HWND m_hwnd;

	Font* m_Font = nullptr;
	FontShader* m_FontShader = nullptr;

	int m_screenWidth = 0;
	int m_screenHeight = 0;

	XMMATRIX m_baseViewMatrix = XMMatrixIdentity();

	SentenceType* m_sentence = nullptr; // πÆ¿Â
	int m_positionX = 0;
	int m_positionY = 0;
	float m_red = 1.0f;
	float m_green = 1.0f;
	float m_blue = 1.0f;
};