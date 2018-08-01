#include "stdafx.h"
#include "Texture.h"
#include "SkyDome.h"


SkyDome::SkyDome()
{
}

SkyDome::SkyDome(const SkyDome& other)
{
}

SkyDome::~SkyDome()
{
}

bool SkyDome::Initialize(ID3D11Device* pDevice, HWND hwnd, char* pFileName, WCHAR* pTextureFilename)
{
	m_hwnd = hwnd;

	// 스카이 돔 모델 정보를 읽어옵니다.
	if (!LoadSkyDomeModel(pFileName))
	{
		return false;
	}

	// 스카이 돔을 정점에 로드하고 렌더링을 위해 인덱스 버퍼를 로드합니다.
	if (!InitializeBuffers(pDevice))
	{
		return false;
	}

	// Load the texture for this model.
	if (!LoadTexture(pDevice, pTextureFilename))
	{
		return false;
	}

	return true;
}

void SkyDome::Shutdown()
{
	// 텍스처를 해제합니다.
	ReleaseTexture();

	// 스카이 돔 렌더링에 사용된 정점 및 인덱스 버퍼를 해제합니다.
	ReleaseBuffers();

	// 스카이 돔 모델을 해제합니다.
	ReleaseSkyDomeModel();
}

void SkyDome::Render(ID3D11DeviceContext* pDeviceContext, float deltaTime, XMFLOAT3 cameraPosition)
{
	// 회전
	m_rotation.y += 0.0005f * deltaTime;
	if (m_rotation.y > 360.0f)
		m_rotation.y -= 360.0f;

	// 스카이 돔을 카메라 위치를 중심으로 변환합니다.
	
	XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(m_rotation.x * XM_RADIAN, m_rotation.y * XM_RADIAN, m_rotation.z * XM_RADIAN);
	XMMATRIX rM = XMMatrixRotationQuaternion(vQ);
	XMMATRIX tM = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

	m_worldMatrix = rM * tM;

	// 스카이 돔을 렌더링 합니다.
	RenderBuffers(pDeviceContext);
}

int SkyDome::GetIndexCount()
{
	return m_indexCount;
}

//GetTexture 함수는 모델 텍스쳐 자원을 반환합니다.텍스쳐 셰이더가 모델을 그리기 위해서는 이 텍스쳐에 접근할 수 있어야 합니다.
ID3D11ShaderResourceView* SkyDome::GetTexture()
{
	return m_Texture->GetTexture();
}

XMMATRIX SkyDome::GetWorldMatrix()
{
	return m_worldMatrix;
}

bool SkyDome::LoadSkyDomeModel(const char* pFileName)
{
	// 모델 파일을 엽니다.
	std::ifstream fin;
	fin.open(pFileName);

	// 파일을 열 수 없으면 종료합니다.
	if (fin.fail())
	{
		return false;
	}

	// 버텍스 카운트의 값까지 읽는다.
	char input = 0;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 버텍스 카운트를 읽는다.
	fin >> m_vertexCount;

	// 인덱스의 수를 정점 수와 같게 설정합니다.
	m_indexCount = m_vertexCount;

	// 읽어 들인 정점 개수를 사용하여 모델을 생성합니다.
	m_model = new SkyDomeModelType[m_vertexCount];
	if (!m_model)
	{
		return false;
	}

	// 데이터의 시작 부분까지 읽는다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// 버텍스 데이터를 읽습니다.
	for (int i = 0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// 모델 파일을 닫는다.
	fin.close();

	return true;
}

void SkyDome::ReleaseSkyDomeModel()
{
	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}
}

bool SkyDome::InitializeBuffers(ID3D11Device* pDevice)
{
	// 정점 배열을 생성합니다.
	SkyDomeVertexType* vertices = new SkyDomeVertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// 인덱스 배열을 생성합니다.
	unsigned long* indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// 정점 배열과 인덱스 배열을 데이터로 로드합니다.
	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
		indices[i] = i;
	}

	// 정점 버퍼의 구조체를 설정한다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SkyDomeVertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 마침내 정점 버퍼를 생성합니다.
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	// 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 하위 리소스 구조에 인덱스 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
	{
		return false;
	}

	// 이제 버텍스와 인덱스 버퍼가 생성되고로드 된 배열을 해제하십시오.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void SkyDome::ReleaseBuffers()
{
	// 인덱스 버퍼를 해제합니다.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// 버텍스 버퍼를 해제한다.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}
void SkyDome::RenderBuffers(ID3D11DeviceContext* pDeviceContext)
{
	// 정점 버퍼 보폭 및 오프셋을 설정합니다.
	unsigned int stride = sizeof(SkyDomeVertexType);
	unsigned int offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 이 꼭지점 버퍼에서 렌더링되어야하는 프리미티브 유형을 설정합니다.이 경우에는 삼각형입니다.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//제공된 파일 이름으로 텍스쳐 개체를 만들고 초기화
bool SkyDome::LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName)
{
	// Create the texture object.
	m_Texture = new Texture;
	if (!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	if (!m_Texture->Initialize(pDevice, m_hwnd, pFileName))
	{
		return false;
	}

	return true;
}

void SkyDome::ReleaseTexture()
{
	// Release the texture object.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}
	return;
}