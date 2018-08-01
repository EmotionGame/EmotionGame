#include "stdafx.h"
#include "Font.h"
#include "FontShader.h"
#include "Text.h"

Text::Text()
{
}

Text::Text(const Text& other)
{
}

Text::~Text()
{
}

bool Text::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, HWND hwnd, int screenWidth, int screenHeight, XMMATRIX baseViewMatrix, 
	char* pFontdataTXT, wchar_t* pFontDDS, char* pSentences, int positionX, int positionY, float red, float green, float blue)
{
	m_hwnd = hwnd;

	m_positionX = positionX;
	m_positionY = positionY;
	m_red = red;
	m_green = green;
	m_blue = blue;

	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	// Create the font object.
	m_Font = new Font;
	if (!m_Font)
	{
		MessageBox(m_hwnd, L"Text.cpp : m_Font = new Font;", L"Error", MB_OK);
		return false;
	}

	// Initialize the font object.
	if (!m_Font->Initialize(pDevice, m_hwnd, pFontdataTXT, pFontDDS))
	{
		MessageBox(m_hwnd, L"Text.cpp : m_Font->Initialize(device, m_hwnd, fontdataTXT, fontDDS)", L"Error", MB_OK);
		return false;
	}

	// Create the font shader object.
	m_FontShader = new FontShader;
	if (!m_FontShader)
	{
		MessageBox(m_hwnd, L"Text.cpp : m_FontShader = new FontShader;", L"Error", MB_OK);
		return false;
	}

	// Initialize the font shader object.
	if (!m_FontShader->Initialize(pDevice, hwnd))
	{
		MessageBox(m_hwnd, L"Text.cpp : m_FontShader->Initialize(device, hwnd)", L"Error", MB_OK);
		return false;
	}

	// Initialize the sentence.
	if (!InitializeSentence(&m_sentence, 256, pDevice))
	{
		MessageBox(m_hwnd, L"Text.cpp : InitializeSentence(&m_sentence, 256, device)", L"Error", MB_OK);
		return false;
	}

	// Now update the sentence vertex buffer with the new string information.
	if (!UpdateSentence(m_sentence, pSentences, pDeviceContext, m_positionX, m_positionY, m_red, m_green, m_blue))
	{
		MessageBox(m_hwnd, L"Text.cpp : UpdateSentence(m_sentence, sentences, deviceContext, m_positionX, m_positionY, m_red, m_green, m_blue)", L"Error", MB_OK);
		return false;
	}

	return true;
}


void Text::Shutdown()
{
	// Release the sentence.
	ReleaseSentence(&m_sentence);

	// Release the font shader object.
	if (m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = nullptr;
	}

	// Release the font object.
	if (m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = nullptr;
	}

	return;
}


bool Text::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	// Draw the sentence.
	if (!RenderSentence(pDeviceContext, m_sentence, worldMatrix, orthoMatrix))
	{
		MessageBox(m_hwnd, L"Text.cpp : RenderSentence(deviceContext, m_sentence, worldMatrix, orthoMatrix)", L"Error", MB_OK);
		return false;
	}

	return true;
}


bool Text::InitializeSentence(SentenceType** ppSentence, int maxLength, ID3D11Device* pDevice)
{
	FontVertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Create a new sentence object.
	*ppSentence = new SentenceType;
	if (!*ppSentence)
	{
		MessageBox(m_hwnd, L"Text.cpp : *sentence = new SentenceType;", L"Error", MB_OK);
		return false;
	}

	// Initialize the sentence buffers to null.
	(*ppSentence)->vertexBuffer = 0;
	(*ppSentence)->indexBuffer = 0;

	// Set the maximum length of the sentence.
	(*ppSentence)->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	(*ppSentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	(*ppSentence)->indexCount = (*ppSentence)->vertexCount;

	// Create the vertex array.
	vertices = new FontVertexType[(*ppSentence)->vertexCount];
	if (!vertices)
	{
		MessageBox(m_hwnd, L"Text.cpp : vertices = new FontVertexType[(*sentence)->vertexCount];", L"Error", MB_OK);
		return false;
	}

	// Create the index array.
	indices = new unsigned long[(*ppSentence)->indexCount];
	if (!indices)
	{
		MessageBox(m_hwnd, L"Text.cpp : indices = new unsigned long[(*sentence)->indexCount];", L"Error", MB_OK);
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(FontVertexType) * (*ppSentence)->vertexCount));

	// Initialize the index array.
	for (i = 0; i<(*ppSentence)->indexCount; i++)
		indices[i] = i;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(FontVertexType) * (*ppSentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	result = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &(*ppSentence)->vertexBuffer);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Text.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);", L"Error", MB_OK);
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*ppSentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &(*ppSentence)->indexBuffer);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Text.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);", L"Error", MB_OK);
		return false;
	}

	// Release the vertex array as it is no longer needed.
	delete[] vertices;
	vertices = 0;

	// Release the index array as it is no longer needed.
	delete[] indices;
	indices = 0;

	return true;
}


bool Text::UpdateSentence(SentenceType* pSentence, char* pText, ID3D11DeviceContext* pDeviceContext, int positionX, int positionY, float red, float green, float blue)
{
	int numLetters;
	FontVertexType* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	FontVertexType* verticesPtr;

	// Store the color of the sentence.
	pSentence->red = red;
	pSentence->green = green;
	pSentence->blue = blue;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(pText);

	// Check for possible buffer overflow.
	if (numLetters > pSentence->maxLength)
	{
		MessageBox(m_hwnd, L"Text.cpp : numLetters > sentence->maxLength", L"Error", MB_OK);
		return false;
	}

	// Create the vertex array.
	vertices = new FontVertexType[pSentence->vertexCount];
	if (!vertices)
	{
		MessageBox(m_hwnd, L"Text.cpp : vertices = new FontVertexType[sentence->vertexCount];", L"Error", MB_OK);
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(FontVertexType) * pSentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray((void*)vertices, pText, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	result = pDeviceContext->Map(pSentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"Text.cpp : deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);", L"Error", MB_OK);
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (FontVertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(FontVertexType) * pSentence->vertexCount));

	// Unlock the vertex buffer.
	pDeviceContext->Unmap(pSentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	delete[] vertices;
	vertices = nullptr;

	return true;
}


void Text::ReleaseSentence(SentenceType** ppSentence)
{
	if (*ppSentence)
	{
		// Release the sentence vertex buffer.
		if ((*ppSentence)->vertexBuffer)
		{
			(*ppSentence)->vertexBuffer->Release();
			(*ppSentence)->vertexBuffer = nullptr;
		}

		// Release the sentence index buffer.
		if ((*ppSentence)->indexBuffer)
		{
			(*ppSentence)->indexBuffer->Release();
			(*ppSentence)->indexBuffer = nullptr;
		}

		// Release the sentence.
		delete *ppSentence;
		*ppSentence = nullptr;
	}

	return;
}


bool Text::RenderSentence(ID3D11DeviceContext* pDeviceContext, SentenceType* pSentence, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	unsigned int stride, offset;
	XMFLOAT4 pixelColor;
	bool result;

	// Set vertex buffer stride and offset.
	stride = sizeof(FontVertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	pDeviceContext->IASetVertexBuffers(0, 1, &pSentence->vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	pDeviceContext->IASetIndexBuffer(pSentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create a pixel color vector with the input sentence color.
	pixelColor = XMFLOAT4(pSentence->red, pSentence->green, pSentence->blue, 1.0f);

	// Render the text using the font shader.
	result = m_FontShader->Render(pDeviceContext, pSentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor);
	if (!result)
	{
		MessageBox(m_hwnd, L"Text.cpp : m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor);", L"Error", MB_OK);
		false;
	}

	return true;
}

bool Text::SetSentenceWithSTR(char* pFrontSentence, char* pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext)
{
	char resultString[64];

	// Convert the mainObjects integer to string format.
	sprintf_s(resultString, "%s%s%s", pFrontSentence, pData, pBackSentence);

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	if (!UpdateSentence(m_sentence, resultString, pDeviceContext, m_positionX, m_positionY, m_red, m_green, m_blue))
	{
		MessageBox(m_hwnd, L"Text.cpp : UpdateSentence(m_sentence, resultString, deviceContext, m_positionX, m_positionY, m_red, m_green, m_blue)", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Text::SetSentenceWithINT(char* pFrontSentence, int pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext)
{
	char resultString[64];

	// Convert the mainObjects integer to string format.
	sprintf_s(resultString, "%s%d%s", pFrontSentence, pData, pBackSentence);

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	if (!UpdateSentence(m_sentence, resultString, pDeviceContext, m_positionX, m_positionY, m_red, m_green, m_blue))
	{
		MessageBox(m_hwnd, L"Text.cpp : UpdateSentence(m_sentence, resultString, deviceContext, m_positionX, m_positionY, m_red, m_green, m_blue)", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Text::SetSentenceWithFLOAT(char* pFrontSentence, float pData, char* pBackSentence, ID3D11DeviceContext* pDeviceContext)
{
	char resultString[64];

	// Convert the mainObjects integer to string format.
	sprintf_s(resultString, "%s%0.2f%s", pFrontSentence, pData, pBackSentence);

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	if (!UpdateSentence(m_sentence, resultString, pDeviceContext, m_positionX, m_positionY, m_red, m_green, m_blue))
	{
		MessageBox(m_hwnd, L"Text.cpp : UpdateSentence(m_sentence, resultString, deviceContext, m_positionX, m_positionY, m_red, m_green, m_blue)", L"Error", MB_OK);
		return false;
	}

	return true;
}