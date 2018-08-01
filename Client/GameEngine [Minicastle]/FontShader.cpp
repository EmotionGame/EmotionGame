#include "stdafx.h"
#include "FontShader.h"

FontShader::FontShader()
{
}

FontShader::FontShader(const FontShader& other)
{
}

FontShader::~FontShader()
{
}

bool FontShader::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
	m_hwnd = hwnd;

	// ���� �� �ȼ� ���̴��� �ʱ�ȭ�մϴ�.
	if (!InitializeShader(pDevice, hwnd, L"HLSL/Font.vs", L"HLSL/Font.ps"))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : InitializeShader", L"Error", MB_OK);
		return false;
	}

	return true;
}

void FontShader::Shutdown()
{
	// ���ؽ� �� �ȼ� ���̴��� ���õ� ��ü�� �����մϴ�.
	ShutdownShader();
}

bool FontShader::Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* pTexture, XMFLOAT4 pixelColor)
{
	// �������� ����� ���̴� �Ű� ������ �����մϴ�.
	if (!SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, pTexture, pixelColor))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor)", L"Error", MB_OK);
		return false;
	}

	// ������ ���۸� ���̴��� �������Ѵ�.
	RenderShader(pDeviceContext, indexCount);

	return true;
}


bool FontShader::InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFilename, WCHAR* pPixelShaderFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	// ���ؽ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(pVertexShaderFilename, NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pVertexShaderFilename);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, pVertexShaderFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// �ȼ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(pPixelShaderFilename, NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pPixelShaderFilename);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, pPixelShaderFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// ���۷κ��� ���� ���̴��� �����Ѵ�.
	result = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);", L"Error", MB_OK);
		return false;
	}

	// ���ۿ��� �ȼ� ���̴��� �����մϴ�.
	result = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);", L"Error", MB_OK);
		return false;
	}

	// ���� �Է� ���̾ƿ� ����ü�� �����մϴ�.
	// �� ������ ModelClass�� ���̴��� VertexType ������ ��ġ�ؾ��մϴ�.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// ���̾ƿ��� ��� ���� �����ɴϴ�.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// ���� �Է� ���̾ƿ��� ����ϴ�.
	result = pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);", L"Error", MB_OK);
		return false;
	}

	// �� �̻� ������ �ʴ� ���� ���̴� �۹��� �ȼ� ���̴� ���۸� �����մϴ�.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ��մϴ�.
	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
	result = pDevice->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);", L"Error", MB_OK);
		return false;
	}

	// �ؽ�ó ���÷� ���� ����ü�� ���� �� �����մϴ�.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// �ؽ�ó ���÷� ���¸� ����ϴ�.
	result = pDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreateSamplerState(&samplerDesc, &m_sampleState);", L"Error", MB_OK);
		return false;
	}

	// Setup the description of the dynamic pixel constant buffer that is in the pixel shader.
	D3D11_BUFFER_DESC pixelBufferDesc;
	pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
	pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelBufferDesc.MiscFlags = 0;
	pixelBufferDesc.StructureByteStride = 0;

	// Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = pDevice->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);
	if (FAILED(result))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);", L"Error", MB_OK);
		return false;
	}

	return true;
}


void FontShader::ShutdownShader()
{
	// Release the pixel constant buffer.
	if (m_pixelBuffer)
	{
		m_pixelBuffer->Release();
		m_pixelBuffer = 0;
	}

	// ���÷� ���¸� �����Ѵ�.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// ��� ��� ���۸� �����մϴ�.
	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = 0;
	}

	// ���̾ƿ��� �����մϴ�.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// �ȼ� ���̴��� �����մϴ�.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// ���ؽ� ���̴��� �����մϴ�.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}


void FontShader::OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFilename)
{
	// ���� �޽����� ���â�� ǥ���մϴ�.
	OutputDebugStringA(reinterpret_cast<const char*>(pErrorMessage->GetBufferPointer()));

	// ���� �޼����� ��ȯ�մϴ�.
	pErrorMessage->Release();
	pErrorMessage = 0;

	// ������ ������ ������ �˾� �޼����� �˷��ݴϴ�.
	MessageBox(hwnd, L"Error compiling shader.", pShaderFilename, MB_OK);
}


bool FontShader::SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* pTexture, XMFLOAT4 pixelColor)
{
	// ��� ������ ������ �� �� �ֵ��� ��޴ϴ�.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(pDeviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	ConstantBufferType* dataPtr = (ConstantBufferType*)mappedResource.pData;

	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� �մϴ�
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ��� ���ۿ� ����� �����մϴ�.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// ��� ������ ����� Ǳ�ϴ�.
	pDeviceContext->Unmap(m_constantBuffer, 0);

	// ���� ���̴������� ��� ������ ��ġ�� �����մϴ�.
	unsigned int bufferNumber = 0;

	// ���������� ���� ���̴��� ��� ���۸� �ٲ� ������ �ٲߴϴ�.
	pDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	// �ȼ� ���̴����� ���̴� �ؽ�ó ���ҽ��� �����մϴ�.
	pDeviceContext->PSSetShaderResources(0, 1, &pTexture);

	// Lock the pixel constant buffer so it can be written to.
	if (FAILED(pDeviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"FontShader.cpp : deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// Get a pointer to the data in the pixel constant buffer.
	PixelBufferType* dataPtr2 = (PixelBufferType*)mappedResource.pData;

	// Copy the pixel color into the pixel constant buffer.
	dataPtr2->pixelColor = pixelColor;

	// Unlock the pixel constant buffer.
	pDeviceContext->Unmap(m_pixelBuffer, 0);

	// Set the position of the pixel constant buffer in the pixel shader.
	bufferNumber = 0;

	// Now set the pixel constant buffer in the pixel shader with the updated value.
	pDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);
	return true;
}


void FontShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount)
{
	// ���� �Է� ���̾ƿ��� �����մϴ�.
	pDeviceContext->IASetInputLayout(m_layout);

	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� �����մϴ�.
	pDeviceContext->VSSetShader(m_vertexShader, NULL, 0);
	pDeviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// �ȼ� ���̴����� ���÷� ���¸� �����մϴ�.
	pDeviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// �ﰢ���� �׸��ϴ�.
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}