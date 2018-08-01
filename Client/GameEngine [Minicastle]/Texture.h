#pragma once

class Texture : public AlignedAllocationPolicy<16>
{
private:
	enum TextureFormat {
		DDS_FORMAT = 1,
		BMP_FORMAT = 2,
		JPEG_FORMAT = 3,
		PNG_FORMAT = 4,
		TIFF_FORMAT = 5,
		GIF_FORMAT = 6
	};

public:
	Texture();
	Texture(const Texture& other);
	~Texture();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, WCHAR* pFileName);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	bool CheckFormat(WCHAR* pFileName);
	bool Last4wcscmp(const WCHAR* pFileName, const WCHAR* pLast4FileName);

private:
	HWND m_hwnd;

	ID3D11ShaderResourceView * m_texture = nullptr;

	unsigned int m_TextureFormat = 0;
};