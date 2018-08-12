#include "stdafx.h"
#include "Line.h"

Line::Line()
{

}
Line::Line(const Line& rOther)
{

}
Line& Line::operator=(const Line& rOther)
{
	m_hwnd = rOther.m_hwnd;

	m_VertexBuffer = rOther.m_VertexBuffer;
	m_IndexBuffer = rOther.m_IndexBuffer;

	m_ColorShader = rOther.m_ColorShader;

	return *this;
}
Line::~Line()
{

}

bool Line::Initialize(ID3D11Device* pDevice, HWND hwnd, XMFLOAT3 vertex[8], XMFLOAT4 color)
{
	m_hwnd = hwnd;

	if (!InitializeBuffers(pDevice, vertex, color))
	{
		MessageBox(m_hwnd, L"Line.cpp : InitializeBuffers(pDevice)", L"Error", MB_OK);
		return false;
	}

	if (!m_ColorShader.Initialize(pDevice, m_hwnd))
	{
		MessageBox(m_hwnd, L"Line.cpp : m_ColorShader->Initialize(device, m_hwnd)", L"Error", MB_OK);
		return false;
	}

	return true;
}
void Line::Shutdown()
{
	m_ColorShader.Shutdown();

	ShutdownBuffers();
}
bool Line::Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool collisionCheck)
{
	RenderBuffers(pDeviceContext);

	if (!m_ColorShader.Render(pDeviceContext, 24, worldMatrix, viewMatrix, projectionMatrix, collisionCheck))
	{
		MessageBox(m_hwnd, L"Line.cpp : m_ColorShader->Render", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Line::InitializeBuffers(ID3D11Device* pDevice, XMFLOAT3 vertex[8], XMFLOAT4 color)
{
	LineVertex* vertices = new LineVertex[8];
	for (int i = 0; i < 8; i++)
	{
		vertices[i].position = vertex[i];
		vertices[i].color = color;
	}

	unsigned int* indices = new unsigned int[24];
	indices[0] = 0;
	indices[1] = 1;

	indices[2] = 1;
	indices[3] = 2;

	indices[4] = 2;
	indices[5] = 3;

	indices[6] = 3;
	indices[7] = 0;

	indices[8] = 0;
	indices[9] = 4;

	indices[10] = 1;
	indices[11] = 5;

	indices[12] = 2;
	indices[13] = 6;

	indices[14] = 3;
	indices[15] = 7;

	indices[16] = 4;
	indices[17] = 5;

	indices[18] = 5;
	indices[19] = 6;

	indices[20] = 6;
	indices[21] = 7;

	indices[22] = 7;
	indices[23] = 4;

	// ���� ���� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(LineVertex) * 8;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ���۸� ����ϴ�.
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer)))
	{
		MessageBox(m_hwnd, L"Line.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer)", L"Error", MB_OK);
		return false;
	}

	// ���� �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * 24;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� �����͸� ����Ű�� ���� ���ҽ� ����ü�� �ۼ��մϴ�.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� �����մϴ�.
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer)))
	{
		MessageBox(m_hwnd, L"Line.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer)", L"Error", MB_OK);
		return false;
	}

	// �����ǰ� ���� �Ҵ�� ���� ���ۿ� �ε��� ���۸� �����մϴ�.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void Line::ShutdownBuffers()
{
	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = nullptr;
	}
}

void Line::RenderBuffers(ID3D11DeviceContext* pDeviceContext)
{
	// ���� ������ ������ �������� �����մϴ�.
	UINT stride = sizeof(LineVertex);
	UINT offset = 0;

	// ������ �� �� �ֵ��� �Է� ��������� ���� ���۸� Ȱ������ �����մϴ�.
	pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	// ������ �� �� �ֵ��� �Է� ��������� �ε��� ���۸� Ȱ������ �����մϴ�.
	pDeviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	/*
	�� ��� : D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
	�׸��� ȣ���� ��� ������ �������� ������ �׷�����.

	�� �� : D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
	�� ���� ��� �׸��� ȣ���� �������� ���ʷ� �̾����� �Ϸ��� ���е��� �׷�����.
	n + 1���� �������� n���� ������ ���������.

	�� ��� : D3D11_PRIMITIVE_TOPOLOGY_LINELIST
	�� ����� ��� �׸��� ȣ���� �� ���� �� ���� �������� �ϳ��� ������ �����Ѵ�.
	����, 2n���� �������� n���� ������ ����� ����.

	�ﰢ�� �� : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	�ﰢ�� ���� ��� �������� ����Ǿ �Ϸ��� �ﰢ������ �����Ѵ�.
	�ﰢ������ ����Ǿ� �ִٴ� ���� �Ͽ��� ������ �� �ﰢ���� �������� �����Ѵ�.
	n���� �������� n - 2���� �ﰢ���� ����� ����.
	���� ������ �� ����� �� �ϴ�.

	�ﰢ�� ��� : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	�ﰢ�� ����� ��� �׸��� ȣ���� �� ���� �� ���� �ϳ��� �������� �ﰢ���� �����Ѵ�.
	����, 3n���� �������� n���� �ﰢ���� ����� ����.

	������ �ﰢ�� ��� : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	���� �ﰢ���鿡 �����ؾ� �ϴ� Ư���� ���� ���̵� �˰����� ������ �� ���δ�.
	*/
}