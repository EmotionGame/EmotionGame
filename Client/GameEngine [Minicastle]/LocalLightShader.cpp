#include "stdafx.h"
#include "LocalLightShader.h"

LocalLightShader::LocalLightShader()
{
}

LocalLightShader::LocalLightShader(const LocalLightShader& rOther)
{
}

LocalLightShader& LocalLightShader::operator=(const LocalLightShader& rOther)
{
	m_hwnd = rOther.m_hwnd;

	m_vertexShader = rOther.m_vertexShader;
	m_pixelShader = rOther.m_pixelShader;
	m_layout = rOther.m_layout;
	m_sampleState = rOther.m_sampleState;

	m_matrixBuffer = rOther.m_matrixBuffer;
	m_cameraBuffer = rOther.m_cameraBuffer;
	m_lightBuffer = rOther.m_lightBuffer;

	return *this;
}

LocalLightShader::~LocalLightShader()
{
}

bool LocalLightShader::Initialize(ID3D11Device* pDevice, HWND hwnd)
{
	m_hwnd = hwnd;

	// 정점 및 픽셀 쉐이더를 초기화합니다.
	return InitializeShader(pDevice, hwnd, const_cast<wchar_t*>(L"HLSL/LocalLight_vs.hlsl"), const_cast<wchar_t*>(L"HLSL/LocalLight_ps.hlsl"));
}

void LocalLightShader::Shutdown()
{
	// 버텍스 및 픽셀 쉐이더와 관련된 객체를 종료합니다.
	ShutdownShader();
}

bool LocalLightShader::Render(ID3D11DeviceContext* pDeviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* pTexture, XMFLOAT3 lightDirection, XMFLOAT3 cameraPosition, XMFLOAT4 ambientColor[], XMFLOAT4 diffuseColor[], XMFLOAT4 specularPower[], XMFLOAT4 specularColor[])
{
	// 렌더링에 사용할 셰이더 매개 변수를 설정합니다.
	if (!SetShaderParameters(pDeviceContext, worldMatrix, viewMatrix, projectionMatrix, pTexture, lightDirection, cameraPosition, ambientColor, diffuseColor, specularPower, specularColor))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambientColor, diffuseColor, cameraPosition, specularColor, specularPower)", L"Error", MB_OK);
		return false;
	}

	// 설정된 버퍼를 셰이더로 렌더링한다.
	RenderShader(pDeviceContext, indexCount);

	return true;
}

bool LocalLightShader::InitializeShader(ID3D11Device* pDevice, HWND hwnd, WCHAR* pVertexShaderFileName, WCHAR* pPixelShaderFileName)
{
	HRESULT hResult;
	ID3D10Blob* errorMessage = nullptr;

	// 버텍스 쉐이더 코드를 컴파일한다.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	hResult = D3DCompileFromFile(pVertexShaderFileName, NULL, NULL, "LocalLightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
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
	hResult = D3DCompileFromFile(pPixelShaderFileName, NULL, NULL, "LocalLightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
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
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);", L"Error", MB_OK);
		return false;
	}

	// 버퍼에서 픽셀 쉐이더를 생성합니다.
	hResult = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);", L"Error", MB_OK);
		return false;
	}

	// 정점 입력 레이아웃 구조체를 설정합니다.
	// 이 설정은 ModelClass와 셰이더의 VertexType 구조와 일치해야합니다.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[4];
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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "BLENDINDICES";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32_UINT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	// 레이아웃의 요소 수를 가져옵니다.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 만듭니다.
	hResult = pDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);", L"Error", MB_OK);
		return false;
	}

	// 더 이상 사용되지 않는 정점 셰이더 퍼버와 픽셀 셰이더 버퍼를 해제합니다.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// 텍스처 샘플러 상태 구조체를 생성 및 설정합니다.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
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

	// 텍스처 샘플러 상태를 만듭니다.
	hResult = pDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateSamplerState(&samplerDesc, &m_sampleState);", L"Error", MB_OK);
		return false;
	}

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
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);", L"Error", MB_OK);
		return false;
	}

	// 버텍스 쉐이더에있는 카메라 동적 상수 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 정점 셰이더 상수 버퍼에 액세스 할 수 있도록 카메라 상수 버퍼 포인터를 만듭니다.
	hResult = pDevice->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);", L"Error", MB_OK);
		return false;
	}

	// 픽셀 쉐이더에있는 광원 동적 상수 버퍼의 설명을 설정합니다.
	// D3D11_BIND_CONSTANT_BUFFER를 사용하면 ByteWidth가 항상 16의 배수 여야하며 그렇지 않으면 CreateBuffer가 실패합니다.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 정점 셰이더 상수 버퍼에 액세스 할 수 있도록 상수 버퍼 포인터를 만듭니다.
	hResult = pDevice->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(hResult))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);", L"Error", MB_OK);
		return false;
	}

	return true;
}

void LocalLightShader::ShutdownShader()
{
	// 광원 상수 버퍼를 해제합니다.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = nullptr;
	}

	// 카메라 상수 버퍼를 해제합니다.
	if (m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = nullptr;
	}

	// 행렬 상수 버퍼를 해제합니다.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}

	// 샘플러 상태를 해제한다.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = nullptr;
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

void LocalLightShader::OutputShaderErrorMessage(ID3D10Blob* pErrorMessage, HWND hwnd, WCHAR* pShaderFileName)
{
	// 에러 메시지를 출력창에 표시합니다.
	OutputDebugStringA(reinterpret_cast<const char*>(pErrorMessage->GetBufferPointer()));

	// 에러 메세지를 반환합니다.
	pErrorMessage->Release();
	pErrorMessage = 0;

	// 컴파일 에러가 있음을 팝업 메세지로 알려줍니다.
	MessageBox(hwnd, L"Error compiling shader.", pShaderFileName, MB_OK);
}

bool LocalLightShader::SetShaderParameters(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* pTexture, XMFLOAT3 lightDirection, XMFLOAT3 cameraPosition, XMFLOAT4 ambientColor[], XMFLOAT4 diffuseColor[], XMFLOAT4 specularPower[], XMFLOAT4 specularColor[])
{
	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 합니다
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(pDeviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
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

	// 쓸 수 있도록 카메라 상수 버퍼를 잠급니다.
	if (FAILED(pDeviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	CameraBufferType* dataPtr1_vs = (CameraBufferType*)mappedResource.pData;

	// 카메라 위치를 상수 버퍼에 복사합니다.
	dataPtr1_vs->cameraPosition = cameraPosition;
	dataPtr1_vs->padding = 0.0f;

	// 카메라 상수 버퍼를 잠금 해제합니다.
	pDeviceContext->Unmap(m_cameraBuffer, 0);

	// 버텍스 쉐이더에서 카메라 상수 버퍼의 위치를 ​​설정합니다.
	bufferNumber = 1;

	// 이제 업데이트 된 값으로 버텍스 쉐이더에서 카메라 상수 버퍼를 설정합니다.
	pDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

	// 픽셀 셰이더에서 셰이더 텍스처 리소스를 설정합니다.
	pDeviceContext->PSSetShaderResources(0, 1, &pTexture);

	// light constant buffer를 잠글 수 있도록 기록한다.
	if (FAILED(pDeviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MessageBox(m_hwnd, L"LocalLightShader.cpp : deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)", L"Error", MB_OK);
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	LightBufferType* dataPtr_ps = (LightBufferType*)mappedResource.pData;

	// 조명 변수를 상수 버퍼에 복사합니다.
	for (int i = 0; i < MATERIAL_SIZE; i++)
		dataPtr_ps->ambientColor[i] = ambientColor[i];

	for (int i = 0; i < MATERIAL_SIZE; i++)
		dataPtr_ps->diffuseColor[i] = diffuseColor[i];

	for (int i = 0; i < MATERIAL_SIZE; i++)
		dataPtr_ps->specularColor[i] = specularColor[i];

	for (int i = 0; i < MATERIAL_SIZE; i++)
		dataPtr_ps->specularPower[i] = specularPower[i]; // x만 사용. yzw는 패딩용...

	dataPtr_ps->lightDirection = lightDirection;


	// 상수 버퍼의 잠금을 해제합니다.
	pDeviceContext->Unmap(m_lightBuffer, 0);

	// 픽셀 쉐이더에서 광원 상수 버퍼의 위치를 ??설정합니다.
	bufferNumber = 0;

	// 마지막으로 업데이트 된 값으로 픽셀 쉐이더에서 광원 상수 버퍼를 설정합니다.
	pDeviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}


void LocalLightShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	pDeviceContext->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	pDeviceContext->VSSetShader(m_vertexShader, NULL, 0);
	pDeviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	pDeviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// 삼각형을 그립니다.
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}