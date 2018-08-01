#pragma once

class Texture;

class SkyDome : public AlignedAllocationPolicy<16>
{
private:
	struct SkyDomeModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct SkyDomeVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

public:
	SkyDome();
	SkyDome(const SkyDome& other);
	~SkyDome();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, char* pFileName, WCHAR* pTextureFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext* pDeviceContext, float deltaTime, XMFLOAT3 cameraPosition);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

	XMMATRIX GetWorldMatrix();

private:
	bool LoadSkyDomeModel(const char* pFileName);
	void ReleaseSkyDomeModel();

	bool InitializeBuffers(ID3D11Device* pDevice);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext* pDeviceContext);
	
	bool LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName);
	void ReleaseTexture();
private:
	HWND m_hwnd;

	SkyDomeModelType * m_model = nullptr;

	int m_vertexCount = 0;
	int m_indexCount = 0;

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;

	Texture* m_Texture = nullptr;

	XMFLOAT3 m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX m_worldMatrix = XMMatrixIdentity();
};