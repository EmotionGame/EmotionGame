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

	// 정적 정점 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(LineVertex) * 8;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer)))
	{
		MessageBox(m_hwnd, L"Line.cpp : device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer)", L"Error", MB_OK);
		return false;
	}

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * 24;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 데이터를 가리키는 보조 리소스 구조체를 작성합니다.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성합니다.
	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer)))
	{
		MessageBox(m_hwnd, L"Line.cpp : device->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer)", L"Error", MB_OK);
		return false;
	}

	// 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다.
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
	// 정점 버퍼의 단위와 오프셋을 설정합니다.
	UINT stride = sizeof(LineVertex);
	UINT offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	// 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	pDeviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	/*
	점 목록 : D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
	그리기 호출의 모든 정점은 개별적인 점으로 그려진다.

	선 띠 : D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
	선 띠의 경우 그리기 호출의 정점들이 차례로 이어져서 일련의 선분들이 그려진다.
	n + 1개의 정점으로 n개의 선분이 만들어진다.

	선 목록 : D3D11_PRIMITIVE_TOPOLOGY_LINELIST
	선 목록의 경우 그리기 호출의 매 정점 두 개가 개별적인 하나의 선분을 형성한다.
	따라서, 2n개의 정점으로 n개의 선분이 만들어 진다.

	삼각형 띠 : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	삼각형 띠의 경우 정점들이 연결되어서 일련의 삼각형들을 형성한다.
	삼각형들이 연결되어 있다는 가정 하에서 인접한 두 삼각형이 정점들을 공유한다.
	n개의 정점으로 n - 2개의 삼각형이 만들어 진다.
	감는 순서가 좀 어려운 듯 하다.

	삼각형 목록 : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	삼각형 목록의 경우 그리기 호출의 매 정점 세 개가 하나의 개별적인 삼각형을 형성한다.
	따라서, 3n개의 정점으로 n개의 삼각형이 만들어 진다.

	인접성 삼각형 목록 : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	인접 삼각형들에 접근해야 하는 특정한 기하 셰이딩 알고리즘을 구현할 때 쓰인다.
	*/
}