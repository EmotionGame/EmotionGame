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

	// 정점 및 픽셀 쉐이더를 초기화합니다.
	return InitializeShader(pDevice, hwnd, const_cast<wchar_t*>(L"HLSL/Color_vs.hlsl"), const_cast<wchar_t*>(L"HLSL/Color_ps.hlsl"));
}

void ColorShader::Shutdown()
{
	// 버텍스 및 픽셀 쉐이더와 관련된 객체를 종료합니다.
	ShutdownShader();
}

bool ColorShader::Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck)
{
	// 렌더링에 사용할 셰이더 매개 변수를 설정합니다.
	if (!SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, collisionCheck))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix)", L"Error", MB_OK);
		return false;
	}

	// 설정된 버퍼를 셰이더로 렌더링한다.
	RenderShader(pDeviceContext, indexCount);

	return true;
}

bool ColorShader::InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName)
{
	HRESULT hResult;
	ID3D10Blob* errorMessage = nullptr;

	// 버텍스 쉐이더 코드를 컴파일한다.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	hResult = D3DCompileFromFile(pVertexShaderFileName, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(hResult))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pVertexShaderFileName);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, pVertexShaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 픽셀 쉐이더 코드를 컴파일한다.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	hResult = D3DCompileFromFile(pPixelShaderFileName, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(hResult))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pPixelShaderFileName);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, pPixelShaderFileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 버퍼로부터 정점 셰이더를 생성한다.
	hResult = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);", L"Error", MB_OK);
		return false;
	}

	// 버퍼에서 픽셀 쉐이더를 생성합니다.
	hResult = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);", L"Error", MB_OK);
		return false;
	}

	// 정점 입력 레이아웃 구조체를 설정합니다.
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

	// 레이아웃의 요소 수를 가져옵니다.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 만듭니다.
	hResult = pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);", L"Error", MB_OK);
		return false;
	}

	// 더 이상 사용되지 않는 정점 셰이더 퍼버와 픽셀 셰이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성합니다.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
	hResult = pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);", L"Error", MB_OK);
		return false;
	}

	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성합니다.
	D3D11_BUFFER_DESC collisionCheckBufferDesc;
	collisionCheckBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	collisionCheckBufferDesc.ByteWidth = sizeof(CollisionCheckBufferType);
	collisionCheckBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	collisionCheckBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	collisionCheckBufferDesc.MiscFlags = 0;
	collisionCheckBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
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
	// 행렬 상수 버퍼를 해제합니다.
	if (m_collisionCheckBuffer)
	{
		m_collisionCheckBuffer->Release();
		m_collisionCheckBuffer = nullptr;
	}

	// 행렬 상수 버퍼를 해제합니다.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}

	// 레이아웃을 해제합니다.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	// 픽셀 쉐이더를 해제합니다.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	// 버텍스 쉐이더를 해제합니다.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
}

void ColorShader::OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName)
{
	// 에러 메시지를 출력창에 표시합니다.
	OutputDebugStringA(reinterpret_cast<const char*>(pErrorMessage->GetBufferPointer()));

	// 에러 메세지를 반환합니다.
	pErrorMessage->Release();
	pErrorMessage = 0;

	// 컴파일 에러가 있음을 팝업 메세지로 알려줍니다.
	MessageBox(hwnd, L"Error compiling shader.", pShaderFileName, MB_OK);
}

bool ColorShader::SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck)
{
	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 합니다
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(pDeviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	MatrixBufferType* dataPtr_vs = (MatrixBufferType*)mappedResource.pData;

	// 상수 버퍼에 행렬을 복사합니다.
	dataPtr_vs->world = worldMatrix;
	dataPtr_vs->view = viewMatrix;
	dataPtr_vs->projection = projectionMatrix;

	// 상수 버퍼의 잠금을 풉니다.
	pDeviceContext->Unmap(m_matrixBuffer, 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정합니다.
	unsigned int bufferNumber = 0;

	// 마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꿉니다.
	pDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	if (FAILED(pDeviceContext->Map(m_collisionCheckBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"ColorShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	CollisionCheckBufferType* dataPtr_ps = (CollisionCheckBufferType*)mappedResource.pData;

	// 상수 버퍼에 행렬을 복사합니다.
	dataPtr_ps->collisionCheck = collisionCheck;
	for (int i = 0; i < 15; i++)
	{
		dataPtr_ps->padding[i] = 0;
	}

	// 상수 버퍼의 잠금을 풉니다.
	pDeviceContext->Unmap(m_collisionCheckBuffer, 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정합니다.
	bufferNumber = 0;

	// 마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꿉니다.
	pDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_collisionCheckBuffer);

	return true;
}


void ColorShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	pDeviceContext->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	pDeviceContext->VSSetShader(m_vertexShader, NULL, 0);
	pDeviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 삼각형을 그립니다.
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}