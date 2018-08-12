#include "stdafx.h"
#include "ColorShader.h"

ColorShader::ColorShader()
{
}
ColorShader::~ColorShader()
{
}

bool ColorShader::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
	m_hwnd = hwnd;

	// ���� �� �ȼ� ���̴��� �ʱ�ȭ�մϴ�.
	return InitializeShader(pDevice, hwnd, const_cast<wchar_t*>(L"HLSL/Color_vs.hlsl"), const_cast<wchar_t*>(L"HLSL/Color_ps.hlsl"));
}

void ColorShader::Shutdown()
{
	// ���ؽ� �� �ȼ� ���̴��� ���õ� ��ü�� �����մϴ�.
	ShutdownShader();
}

bool ColorShader::Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck)
{
	// �������� ����� ���̴� �Ű� ������ �����մϴ�.
	if (!SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, collisionCheck))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix)", L"Error", MB_OK);
		return false;
	}

	// ������ ���۸� ���̴��� �������Ѵ�.
	RenderShader(pDeviceContext, indexCount);

	return true;
}

bool ColorShader::InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName)
{
	HRESULT hResult;
	ID3D10Blob* errorMessage = nullptr;

	// ���ؽ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	hResult = D3DCompileFromFile(pVertexShaderFileName, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(hResult))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pVertexShaderFileName);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, pVertexShaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// �ȼ� ���̴� �ڵ带 �������Ѵ�.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	hResult = D3DCompileFromFile(pPixelShaderFileName, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(hResult))
	{
		// ���̴� ������ ���н� �����޽����� ����մϴ�.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pPixelShaderFileName);
		}
		// ������ ������ �ƴ϶�� ���̴� ������ ã�� �� ���� ����Դϴ�.
		else
		{
			MessageBox(hwnd, pPixelShaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// ���۷κ��� ���� ���̴��� �����Ѵ�.
	hResult = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);", L"Error", MB_OK);
		return false;
	}

	// ���ۿ��� �ȼ� ���̴��� �����մϴ�.
	hResult = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);", L"Error", MB_OK);
		return false;
	}

	// ���� �Է� ���̾ƿ� ����ü�� �����մϴ�.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// ���̾ƿ��� ��� ���� �����ɴϴ�.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// ���� �Է� ���̾ƿ��� ����ϴ�.
	hResult = pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);", L"Error", MB_OK);
		return false;
	}

	// �� �̻� ������ �ʴ� ���� ���̴� �۹��� �ȼ� ���̴� ���۸� �����մϴ�.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ��մϴ�.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
	hResult = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);", L"Error", MB_OK);
		return false;
	}

	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ��մϴ�.
	D3D11_BUFFER_DESC collisionCheckBufferDesc;
	collisionCheckBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	collisionCheckBufferDesc.ByteWidth = sizeof(CollisionCheckBufferType);
	collisionCheckBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	collisionCheckBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	collisionCheckBufferDesc.MiscFlags = 0;
	collisionCheckBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
	hResult = pDevice->CreateBuffer(&collisionCheckBufferDesc, NULL, &m_collisionCheckBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateBuffer(&collisionCheckBufferDesc, NULL, &m_collisionCheckBuffer);", L"Error", MB_OK);
		return false;
	}

	return true;
}

void ColorShader::ShutdownShader()
{
	// ��� ��� ���۸� �����մϴ�.
	if (m_collisionCheckBuffer)
	{
		m_collisionCheckBuffer->Release();
		m_collisionCheckBuffer = nullptr;
	}

	// ��� ��� ���۸� �����մϴ�.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}

	// ���̾ƿ��� �����մϴ�.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	// �ȼ� ���̴��� �����մϴ�.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	// ���ؽ� ���̴��� �����մϴ�.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
}

void ColorShader::OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName)
{
	// ���� �޽����� ���â�� ǥ���մϴ�.
	OutputDebugStringA(reinterpret_cast<const char*>(pErrorMessage->GetBufferPointer()));

	// ���� �޼����� ��ȯ�մϴ�.
	pErrorMessage->Release();
	pErrorMessage = 0;

	// ������ ������ ������ �˾� �޼����� �˷��ݴϴ�.
	MessageBox(hwnd, L"Error compiling shader.", pShaderFileName, MB_OK);
}

bool ColorShader::SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck)
{
	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� �մϴ�
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ��� ������ ������ �� �� �ֵ��� ��޴ϴ�.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(pDeviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
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

	// ��� ������ ������ �� �� �ֵ��� ��޴ϴ�.
	if (FAILED(pDeviceContext->Map(m_collisionCheckBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� �����ɴϴ�.
	CollisionCheckBufferType* dataPtr_ps = (CollisionCheckBufferType*)mappedResource.pData;

	// ��� ���ۿ� ����� �����մϴ�.
	dataPtr_ps->collisionCheck = collisionCheck;
	for (int i = 0; i < 15; i++)
	{
		dataPtr_ps->padding[i] = 0;
	}

	// ��� ������ ����� Ǳ�ϴ�.
	pDeviceContext->Unmap(m_collisionCheckBuffer, 0);

	// ���� ���̴������� ��� ������ ��ġ�� �����մϴ�.
	bufferNumber = 0;

	// ���������� ���� ���̴��� ��� ���۸� �ٲ� ������ �ٲߴϴ�.
	pDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_collisionCheckBuffer);

	return true;
}


void ColorShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount)
{
	// ���� �Է� ���̾ƿ��� �����մϴ�.
	pDeviceContext->IASetInputLayout(m_layout);

	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� �����մϴ�.
	pDeviceContext->VSSetShader(m_vertexShader, NULL, 0);
	pDeviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// �ﰢ���� �׸��ϴ�.
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}