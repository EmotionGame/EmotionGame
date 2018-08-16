#include "stdafx.h"
#include "EmotionShader.h"

EmotionShader::EmotionShader()
{
}

EmotionShader::EmotionShader(const EmotionShader& rOther)
{
}

EmotionShader::~EmotionShader()
{
}

bool EmotionShader::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
	m_hwnd = hwnd;

	// Initialize the vertex and pixel shaders.
	if (!InitializeShader(pDevice, hwnd, const_cast<wchar_t*>(L"HLSL/Emotion_vs.hlsl"), const_cast<wchar_t*>(L"HLSL/Emotion_ps.hlsl")))
	{
		return false;
	}

	return true;
}

void EmotionShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool EmotionShader::Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* pTexture, int maxEmotionValue, int maxEmotionKind, int screenWidth, int screenHeight)
{
	// Set the shader parameters that it will use for rendering.
	if (!SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, pTexture, maxEmotionValue, maxEmotionKind, screenWidth, screenHeight))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, pTexture)", L"Error", MB_OK);
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(pDeviceContext, indexCount);

	return true;
}

bool EmotionShader::InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName)
{
	ID3D10Blob* errorMessage = nullptr;

	// Compile the vertex shader code.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(pVertexShaderFileName, NULL, NULL, "EmotionVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage)))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pVertexShaderFileName);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, pVertexShaderFileName, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	// Compile the pixel shader code.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(pPixelShaderFileName, NULL, NULL, "EmotionPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage)))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pPixelShaderFileName);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, pPixelShaderFileName, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	// Create the vertex shader from the buffer.
	if (FAILED(pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);", L"Error", MB_OK);
		return false;
	}

	// Create the pixel shader from the buffer.
	if (FAILED(pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);", L"Error", MB_OK);
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
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

	// Get a count of the elements in the layout.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	if (FAILED(pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);", L"Error", MB_OK);
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;
		
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	if (FAILED(pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);", L"Error", MB_OK);
		return false;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC emotionBufferDesc;
	emotionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	emotionBufferDesc.ByteWidth = sizeof(EmotionBufferType);
	emotionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	emotionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	emotionBufferDesc.MiscFlags = 0;
	emotionBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	if (FAILED(pDevice->CreateBuffer(&emotionBufferDesc, NULL, &m_emotionBuffer)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : pDevice->CreateBuffer(&emotionBufferDesc, NULL, &m_emotionBuffer);", L"Error", MB_OK);
		return false;
	}

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // ����
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 3���� �𵨿� �ؽ��ĸ� ���� �� ��� ������ ������
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

	// Create the texture sampler state.
	if (FAILED(pDevice->CreateSamplerState(&samplerDesc, &m_sampleState)))
	{
		MessageBox(m_hwnd, L"EmotionShader.cpp : pDevice->CreateSamplerState(&samplerDesc, &m_sampleState);", L"Error", MB_OK);
		return false;
	}

	return true;
}

void EmotionShader::ShutdownShader()
{
	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void EmotionShader::OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName)
{
	// ���� �޽����� ���â�� ǥ���մϴ�.
	OutputDebugStringA(reinterpret_cast<const char*>(pErrorMessage->GetBufferPointer()));

	// ���� �޼����� ��ȯ�մϴ�.
	pErrorMessage->Release();
	pErrorMessage = 0;

	// ������ ������ ������ �˾� �޼����� �˷��ݴϴ�.
	MessageBox(hwnd, L"Error compiling shader.", pShaderFileName, MB_OK);
}

bool EmotionShader::SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* pTexture, int maxEmotionValue, int maxEmotionKind, int screenWidth, int screenHeight)
{
	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� �մϴ�
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ��� ������ ������ �� �� �ֵ��� ��޴ϴ�.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(pDeviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"DelayLoadingShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	MatrixBufferType* dataPtr_vs = (MatrixBufferType*)mappedResource.pData;

	// ��� ���ۿ� ����� �����մϴ�.
	dataPtr_vs->world = worldMatrix;
	dataPtr_vs->view = viewMatrix;
	dataPtr_vs->projection = projectionMatrix;

	// ��� ������ ����� Ǳ�ϴ�.
	pDeviceContext->Unmap(m_matrixBuffer, 0);

	// ���� ���̴������� ��� ������ ��ġ�� �����մϴ�.
	unsigned int bufferNumber = 0;

	// ���������� ���� ���̴��� ��� ���۸� �ٲ� ������ �ٲߴϴ�.
	pDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// �ȼ� ���̴����� ���̴� �ؽ�ó ���ҽ��� �����մϴ�.
	pDeviceContext->PSSetShaderResources(0, 1, &pTexture);

	// light constant buffer�� ��� �� �ֵ��� ����Ѵ�.
	if (FAILED(pDeviceContext->Map(m_emotionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	EmotionBufferType* dataPtr_ps = (EmotionBufferType*)mappedResource.pData;
	dataPtr_ps->maxEmotionValue = maxEmotionValue;
	dataPtr_ps->maxEmotionKind = maxEmotionKind;
	dataPtr_ps->screenWidth = screenWidth;
	dataPtr_ps->screenHeight = screenHeight;

	// ��� ������ ����� �����մϴ�.
	pDeviceContext->Unmap(m_emotionBuffer, 0);

	// �ȼ� ���̴����� ���� ��� ������ ��ġ�� ??�����մϴ�.
	bufferNumber = 0;

	// ���������� ������Ʈ �� ������ �ȼ� ���̴����� ���� ��� ���۸� �����մϴ�.
	pDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_emotionBuffer);

	return true;
}


void EmotionShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount)
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