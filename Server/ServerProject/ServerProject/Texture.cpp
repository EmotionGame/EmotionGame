#include "stdafx.h"
#include "Texture.h"

Texture::Texture()
{
}


Texture::Texture(const Texture& other)
{
}


Texture::~Texture()
{
}

bool Texture::Initialize(ID3D11Device* pDevice, HWND hwnd, WCHAR* pFileName)
{
	m_hwnd = hwnd;

	CheckFormat(pFileName);

	switch (m_TextureFormat)
	{
	case DDS_FORMAT:
	{	
		// DDS 텍스처를 파일로부터 읽어온다
		if (FAILED(CreateDDSTextureFromFile(pDevice, pFileName, nullptr, &m_texture)))
		{
			MessageBox(m_hwnd, L"Texture.cpp : CreateDDSTextureFromFile(pDevice, pFileName, nullptr, &m_texture)", L"Error", MB_OK);
			return false;
		}
	}
	break;
	case BMP_FORMAT:
	case JPEG_FORMAT:
	case PNG_FORMAT:
	case TIFF_FORMAT:
	case GIF_FORMAT:
	{
		// WIC 텍스처를 파일로부터 읽어온다
		if (FAILED(CreateWICTextureFromFile(pDevice, pFileName, nullptr, &m_texture)))
		{
			MessageBox(m_hwnd, L"Texture.cpp : CreateWICTextureFromFile(pDevice, pFileName, nullptr, &m_texture)", L"Error", MB_OK);
			return false;
		}
		break;
	}
	}
	return true;
}


void Texture::Shutdown()
{
	//텍스처 뷰 리소스를 해제한다.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}
}


ID3D11ShaderResourceView* Texture::GetTexture()
{
	return m_texture;
}


bool Texture::CheckFormat(WCHAR* pFileName) {
	if (Last4wcscmp(pFileName, L".dds"))
	{
		m_TextureFormat = DDS_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".bmp"))
	{
		m_TextureFormat = BMP_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".jpeg"))
	{
		m_TextureFormat = JPEG_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".jpg"))
	{
		m_TextureFormat = JPEG_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".png"))
	{
		m_TextureFormat = PNG_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".tiff"))
	{
		m_TextureFormat = TIFF_FORMAT;
		return true;
	}
	if (Last4wcscmp(pFileName, L".gif"))
	{
		m_TextureFormat = GIF_FORMAT;
		return true;
	}

	// 지원하는 포맷이 없으면 false 반환
	return false;
}

// 확장자 비교용 함수
bool Texture::Last4wcscmp(const WCHAR* pFileName, const WCHAR* pLast4FileName) {
	int filenameLen = wcslen(pFileName);
	WCHAR last[5];

	last[0] = pFileName[filenameLen - 4];
	last[1] = pFileName[filenameLen - 3];
	last[2] = pFileName[filenameLen - 2];
	last[3] = pFileName[filenameLen - 1];
	last[4] = '\0';

	if (wcscmp(last, pLast4FileName) == 0)
		return true;

	return false;
}