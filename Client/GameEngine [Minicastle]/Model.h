#pragma once

class Texture;
class LocalLightShader;
class LocalLightAnimationShader;
class HID;

class Model : public AlignedAllocationPolicy<16>
{
private:
	struct Info
	{
		bool hasMaterial = 0;
		unsigned int meshCount = 0;
		unsigned int animStackCount = 0;
		unsigned int poseCount = 0;
		unsigned int boneCount = 0;
		unsigned int clusterCount = 0;
		unsigned int totalVertexCount = 0;
		unsigned int totalPolygoncount = 0;
	};
	struct Vertex
	{
		XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);

	};
	struct VertexAnim
	{
		XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);
		unsigned int boneIndex[4] = { 0, 0, 0, 0 };
		float boneWeight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};
	struct polygon
	{
		unsigned int indices[3] = { 0, 0, 0 };
		unsigned int materialIndex = 0;
	};
	struct Material
	{
		unsigned int index = 0;
		bool lambert = false;
		bool phong = true;
		XMFLOAT3 ambient = XMFLOAT3(0.05f, 0.05f, 0.05f);
		XMFLOAT3 diffuse = XMFLOAT3(0.5f, 0.5f, 0.5f);
		XMFLOAT3 specular = XMFLOAT3(0.2f, 0.2f, 0.2f);
		XMFLOAT3 emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 reflection = XMFLOAT3(0.0f, 0.0f, 0.0f);
		float shininess = 64.0f;
		float reflectionFactor = 0.0f;
		float specularPower = 1.0f;
		float transparencyFactor = 0.0f;
	};
	struct GlobalOffPosition
	{
		XMMATRIX globalOffPosition = XMMatrixIdentity();;
	};
	struct ClusterEachFrame
	{
		unsigned int index = 0;
		XMMATRIX finalTransform = XMMatrixIdentity();;
	};

private:
	bool LoadFBX2KSM(char* pFileName);

	bool LoadFBXInfo(char* pFileName);
	bool LoadHasAnimation(char* pFileName);
	bool LoadVertex(char* pFileName, unsigned int meshCount);
	bool LoadVertexAnim(char* pFileName, unsigned int meshCount);
	bool LoadPolygon(char* pFileName, unsigned int meshCount);
	bool LoadMaterial(char* pFileName, unsigned int meshCount);
	bool LoadGlobalOffPosition(char* pFileName);
	bool LoadFinalTransform(char* pFileName, unsigned int meshCount, unsigned int animStackCount);

public:
	Model();
	Model(const Model& other);
	~Model();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, char* pModelFileName, WCHAR* pTextureFileName,
		XMFLOAT3 modelcaling, XMFLOAT3 modelRotation, XMFLOAT3 modelTranslation, bool specularZero, unsigned int animStackNum);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float deltaTime);

	void MoveObejctToLookAt(XMFLOAT3 value);
	void MoveObjectToLookAtUp(XMFLOAT3 value);
	void MoveObejctToLookAtSide(XMFLOAT3 value);
	void RotateObject(XMFLOAT3 value);
	void PlayerControl(float frameTime);

	void SetPosition(XMFLOAT3 position);
	XMFLOAT3 GetPosition();
	void SetRotation(XMFLOAT3 position);
	XMFLOAT3 GetRotation();
	void CalculateCameraPosition();
	XMFLOAT3 GetCameraPosition();

	bool IsActive();
	bool IsInitilized();
	void SetInitStarted(bool initStart);
	bool GetInitStarted();

private:
	bool LoadModel(char* pFileName);
	bool InitializeBuffers(ID3D11Device* pDevice, unsigned int meshCount);
	bool InitializeAnimationBuffers(ID3D11Device* pDevice, unsigned int meshCount);
	bool LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName);
	void RenderBuffers(ID3D11DeviceContext* pDeviceContext, unsigned int meshCount);

	void ReleaseTexture();
	void ShutdownBuffers();

	bool CheckFormat(char* pFileName);
	bool Last4strcmp(const char* pFileName, const char* pLast4FileName);

	XMMATRIX CalculateWorldMatrix();

private:
	HWND m_hwnd;

	bool m_ActiveFlag = true;

	std::unordered_map<unsigned int, ID3D11Buffer*>* m_VertexBuffer = new std::unordered_map<unsigned int, ID3D11Buffer*>;
	std::unordered_map<unsigned int, ID3D11Buffer*>* m_IndexBuffer = new std::unordered_map<unsigned int, ID3D11Buffer*>;

	std::unordered_map<unsigned int, ID3D11Buffer*>* m_AnimationVertexBuffer = new std::unordered_map<unsigned int, ID3D11Buffer*>;
	std::unordered_map<unsigned int, ID3D11Buffer*>* m_AnimationIndexBuffer = new std::unordered_map<unsigned int, ID3D11Buffer*>;

	XMMATRIX m_worldMatrix = XMMatrixIdentity(); // 단위행렬로 초기화

	/***** 제어 : 시작 *****/
	XMFLOAT3 m_ModelScaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 m_ModelRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_ModelTranslation = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT3 m_DefaultLootAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_DefaultUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 m_DefaultSide = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT3 m_LootAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 m_Side = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT3 m_cameraPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float m_limitAngle = 60.0f;
	/***** 제어 : 종료 *****/

	LocalLightShader* m_LocalLightShader = nullptr;
	LocalLightAnimationShader* m_LocalLightAnimationShader = nullptr;

	Texture* m_Texture = nullptr;

	/***** KSM에서 로드한 데이터들 : 시작 *****/
#define BUFFER_SIZE 4096

	Info m_Info;
	std::vector<bool> m_HasAnimation;
	std::unordered_map<unsigned int, std::vector<Vertex>> m_Vertex;
	std::unordered_map<unsigned int, std::vector<VertexAnim>> m_VertexAnim;
	std::vector<std::vector<polygon>> m_polygon;
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, Material>> m_Material;
	std::vector<XMMATRIX> m_GlobalOffPosition;

	struct ClusterMatrix
	{
		std::vector<XMMATRIX> finalTransform;
	};
	struct Animation
	{
		// std::unordered_map<클러스터 번호, 해당 클러스터의 각 프레임마다의 최종 변환을 저장하는 행렬>
		std::unordered_map<unsigned int, ClusterMatrix> m_Animation;
	};
	// std::unordered_map<메시 번호, 애니메이션 스택에 따른 각 애니메이션 벡터>
	std::unordered_map<unsigned int, std::vector<Animation>> m_Animations;
	/***** KSM에서 로드한 데이터들 : 종료 *****/

	/***** Animation 관리 : 시작 *****/
	unsigned int m_AnimStackIndex = 0;
	unsigned int m_AnimFrameCount = 0;
	float m_SumDeltaTime = 0.0f;
	XMMATRIX m_FinalTransform[BONE_FINAL_TRANSFORM_SIZE];
	/***** Animation 관리 : 종료 *****/

	/***** Material 관리 : 시작 *****/
	bool m_SpecularZero = false; // 스펙큘러를 없애고 싶으면 true
	XMFLOAT4 m_AmbientColor[MATERIAL_SIZE];
	XMFLOAT4 m_DiffuseColor[MATERIAL_SIZE];
	XMFLOAT4 m_SpecularPower[MATERIAL_SIZE];
	XMFLOAT4 m_SpecularColor[MATERIAL_SIZE];
	XMFLOAT3 m_LightDirection = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	/***** Material 관리 : 종료 *****/

	HID* m_HID = nullptr; // 포인터를 받아와서 사용하므로 m_HID->Shutdown() 금지

	std::mutex m_InitMutex;
	bool m_Initilized = false;
	
	std::mutex m_InitStartMutex;
	bool m_InitSatrt = false;
};
