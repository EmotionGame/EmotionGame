#pragma once

class Texture;

class Font : public AlignedAllocationPolicy<16>
{
private:
	struct FontType
	{
		float left, right;
		float top, bottom;
		int sizeX;
		int sizeY;
	};
	struct FontVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Font();
	Font(const Font& other);
	~Font();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, char* pFontFileName, WCHAR* pTextureFileName);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	void BuildVertexArray(void* pVertices, char* pSentence, float drawX, float drawY);

private:
	bool LoadFontData(char* pFontFileName);
	void ReleaseFontData();
	bool LoadTexture(ID3D11Device* pDevice, WCHAR* pTextureFileName);
	void ReleaseTexture();

private:
	HWND m_hwnd;

#define FONT_SIZE 95
	FontType * m_Font = nullptr;
	Texture* m_Texture = nullptr;
};