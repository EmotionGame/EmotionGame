#include "stdafx.h"
#include "Texture.h"
#include "Font.h"

Font::Font()
{
}
Font::Font(const Font& other)
{
}
Font::~Font()
{
}

bool Font::Initialize(ID3D11Device* pDevice, HWND hwnd, char* pFontFileName, WCHAR* pTextureFileName)
{
	m_hwnd = hwnd;

	// Load in the text file containing the font data.
	if (!LoadFontData(pFontFileName))
	{
		MessageBox(m_hwnd, L"Font.cpp : LoadFontData(fontFilename)", L"Error", MB_OK);
		return false;
	}

	// Load the texture that has the font characters on it.
	if (!LoadTexture(pDevice, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Font.cpp : LoadTexture(device, textureFilename)", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Font::Shutdown()
{
	// Release the font texture.
	ReleaseTexture();

	// Release the font data.
	ReleaseFontData();
}

bool Font::LoadFontData(char* pFontFileName)
{
	std::ifstream fin;
	int i;
	char temp;

	// Create the font spacing buffer.
	m_Font = new FontType[FONT_SIZE];
	if (!m_Font)
	{
		MessageBox(m_hwnd, L"Font.cpp : m_Font = new FontType[95];", L"Error", MB_OK);
		return false;
	}

	// Read in the font size and spacing between chars.
	fin.open(pFontFileName);
	if (fin.fail())
	{
		MessageBox(m_hwnd, L"Font.cpp : fin.open(filename);", L"Error", MB_OK);
		return false;
	}

	// Read in the 95 used ascii characters for text.
	for (i = 0; i < FONT_SIZE; i++)
	{
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].top;
		fin >> m_Font[i].bottom;
		fin >> m_Font[i].sizeX;
		fin >> m_Font[i].sizeY;
	}

	// Close the file.
	fin.close();

	return true;
}

void Font::ReleaseFontData()
{
	// Release the font data array.
	if (m_Font)
	{
		delete[] m_Font;
		m_Font = nullptr;
	}
}

bool Font::LoadTexture(ID3D11Device* pDevice, WCHAR* pTextureFileName)
{
	// Create the texture object.
	m_Texture = new Texture;
	if (!m_Texture)
	{
		MessageBox(m_hwnd, L"Font.cpp : m_Texture = new Texture;", L"Error", MB_OK);
		return false;
	}

	// Initialize the texture object.
	if (!m_Texture->Initialize(pDevice, m_hwnd, pTextureFileName))
	{
		MessageBox(m_hwnd, L"Font.cpp : m_Texture->Initialize(device, m_hwnd, filename)", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Font::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}

	return;
}

ID3D11ShaderResourceView* Font::GetTexture()
{
	return m_Texture->GetTexture();
}

void Font::BuildVertexArray(void* pVertices, char* pSentence, float drawX, float drawY)
{
	FontVertexType* vertexPtr;
	int numLetters, index, i, letter;

	// Coerce the input vertices into a VertexType structure.
	vertexPtr = (FontVertexType*)pVertices;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(pSentence);

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each letter onto a quad.
	for (i = 0; i<numLetters; i++)
	{
		letter = ((int)pSentence[i]) - 32;

		// If the letter is a space then just move over three pixels.
		if (letter == 0)
		{
			drawX = drawX + 3.0f;
		}
		else
		{
			// First triangle in quad.
			// Top left.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, m_Font[letter].top);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].sizeX), (drawY - m_Font[letter].sizeY), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, m_Font[letter].bottom);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - m_Font[letter].sizeY), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, m_Font[letter].bottom);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, m_Font[letter].top);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].sizeX), drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, m_Font[letter].top);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].sizeX), (drawY - m_Font[letter].sizeY), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, m_Font[letter].bottom);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].sizeX + 1.0f;
		}
	}
}

