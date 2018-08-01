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

	// ��ī�� �� �� ������ �о�ɴϴ�.
	if (!LoadSkyDomeModel(pFileName))
	{
		return false;
	}

	// ��ī�� ���� ������ �ε��ϰ� �������� ���� �ε��� ���۸� �ε��մϴ�.
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
	// �ؽ�ó�� �����մϴ�.
	ReleaseTexture();

	// ��ī�� �� �������� ���� ���� �� �ε��� ���۸� �����մϴ�.
	ReleaseBuffers();

	// ��ī�� �� ���� �����մϴ�.
	ReleaseSkyDomeModel();
}

void SkyDome::Render(ID3D11DeviceContext* pDeviceContext, float deltaTime, XMFLOAT3 cameraPosition)
{
	// ȸ��
	m_rotation.y += 0.0005f * deltaTime;
	if (m_rotation.y > 360.0f)
		m_rotation.y -= 360.0f;

	// ��ī�� ���� ī�޶� ��ġ�� �߽����� ��ȯ�մϴ�.
	
	XMVECTOR vQ = XMQuaternionRotationRollPitchYaw(m_rotation.x * XM_RADIAN, m_rotation.y * XM_RADIAN, m_rotation.z * XM_RADIAN);
	XMMATRIX rM = XMMatrixRotationQuaternion(vQ);
	XMMATRIX tM = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

	m_worldMatrix = rM * tM;

	// ��ī�� ���� ������ �մϴ�.
	RenderBuffers(pDeviceContext);
}

int SkyDome::GetIndexCount()
{
	return m_indexCount;
}

//GetTexture �Լ��� �� �ؽ��� �ڿ��� ��ȯ�մϴ�.�ؽ��� ���̴��� ���� �׸��� ���ؼ��� �� �ؽ��Ŀ� ������ �� �־�� �մϴ�.
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
	// �� ������ ���ϴ�.
	std::ifstream fin;
	fin.open(pFileName);

	// ������ �� �� ������ �����մϴ�.
	if (fin.fail())
	{
		return false;
	}

	// ���ؽ� ī��Ʈ�� ������ �д´�.
	char input = 0;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// ���ؽ� ī��Ʈ�� �д´�.
	fin >> m_vertexCount;

	// �ε����� ���� ���� ���� ���� �����մϴ�.
	m_indexCount = m_vertexCount;

	// �о� ���� ���� ������ ����Ͽ� ���� �����մϴ�.
	m_model = new SkyDomeModelType[m_vertexCount];
	if (!m_model)
	{
		return false;
	}

	// �������� ���� �κб��� �д´�.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// ���ؽ� �����͸� �н��ϴ�.
	for (int i = 0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// �� ������ �ݴ´�.
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
	// ���� �迭�� �����մϴ�.
	SkyDomeVertexType* vertices = new SkyDomeVertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// �ε��� �迭�� �����մϴ�.
	unsigned long* indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// ���� �迭�� �ε��� �迭�� �����ͷ� �ε��մϴ�.
	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
		indices[i] = i;
	}

	// ���� ������ ����ü�� �����Ѵ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SkyDomeVertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ��ħ�� ���� ���۸� �����մϴ�.
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	// �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// ���� ���ҽ� ������ �ε��� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� �����մϴ�.
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
	{
		return false;
	}

	// ���� ���ؽ��� �ε��� ���۰� �����ǰ�ε� �� �迭�� �����Ͻʽÿ�.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void SkyDome::ReleaseBuffers()
{
	// �ε��� ���۸� �����մϴ�.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// ���ؽ� ���۸� �����Ѵ�.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}
void SkyDome::RenderBuffers(ID3D11DeviceContext* pDeviceContext)
{
	// ���� ���� ���� �� �������� �����մϴ�.
	unsigned int stride = sizeof(SkyDomeVertexType);
	unsigned int offset = 0;

	// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
	pDeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
	pDeviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// �� ������ ���ۿ��� �������Ǿ���ϴ� ������Ƽ�� ������ �����մϴ�.�� ��쿡�� �ﰢ���Դϴ�.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//������ ���� �̸����� �ؽ��� ��ü�� ����� �ʱ�ȭ
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