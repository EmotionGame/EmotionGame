#pragma once
#include "Material.h"
#include "Utilities.h"

class Texture;
class LocalLightShader;
class LocalLightAnimationShader;
class FBX;
class HID;

class Model : public AlignedAllocationPolicy<16>
{
private:
	enum ModelFormat {
		FBX_FORMAT = 1,
		OBJ_FORMAT = 2
	};

public:
	Model();
	Model(const Model& other);
	~Model();

	bool Initialize(ID3D11Device* pDevice, HWND hwnd, HID* pHID, char* pModelFileName, WCHAR* pTextureFileName,
		XMFLOAT3 modelcaling, XMFLOAT3 modelRotation, XMFLOAT3 modelTranslation, bool specularZero);
	void Shutdown();
	bool Render(ID3D11DeviceContext* pDeviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPosition, float frameTime);

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

private:
	bool LoadModel(char* pFileName);
	bool InitializeBuffers(ID3D11Device* pDevice);
	bool InitializeAnimationBuffers(ID3D11Device* pDevice);
	bool LoadTexture(ID3D11Device* pDevice, WCHAR* pFileName);
	void RenderBuffers(ID3D11DeviceContext* pDeviceContext, unsigned int i);

	void ReleaseTexture();
	void ShutdownBuffers();

	bool CheckFormat(char* pFileName);
	bool Last4strcmp(const char* pFileName, const char* pLast4FileName);

	XMMATRIX CalculateWorldMatrix();
	XMMATRIX ConvertFbxAMatrixtoXMMATRIX(FbxAMatrix fbxAMatrix);

private:
	HWND m_hwnd;

	bool m_ActiveFlag = true;

	std::vector<ID3D11Buffer*>* m_VertexBuffer = new std::vector<ID3D11Buffer*>;
	std::vector<ID3D11Buffer*>* m_IndexBuffer = new std::vector<ID3D11Buffer*>;

	std::vector<ID3D11Buffer*>* m_AnimationVertexBuffer = new std::vector<ID3D11Buffer*>;
	std::vector<ID3D11Buffer*>* m_AnimationIndexBuffer = new std::vector<ID3D11Buffer*>;

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

	/***** FBX에서 로드한 데이터들 : 시작 *****/
	std::vector<Animation*>* m_AnimationStackVector;
	std::vector<FbxNode*>* m_CameraNodeVector;
	std::vector<FbxPose*>* m_PoseVector;

	Skeleton* m_Skeleton;
	bool m_HasAnimation;

	unsigned int m_MeshCount;
	std::vector<FbxNode*>* m_MeshNodeVector;
	std::vector<bool>* m_HasNormalVector;
	std::vector<bool>* m_HasUVVector;
	std::vector<unsigned int>* m_TriangleCountVector;
	std::vector<TrianglePolygonVector*>* m_TrianglesVector;
	std::vector<PNTIWVertexVector*>* m_VerticesVector;

	std::vector<MaterialUnorderedMap*>* m_MaterialLookUpVector;

	std::vector<LightCache*>* m_LightCacheVector;
	/***** FBX에서 로드한 데이터들 : 종료 *****/

	/***** Animation 관리 : 시작 *****/
	FbxTime mCurrentTime, mFrameTime;
	float m_SumTime = 0.0f;
	unsigned int m_AnimStackIndex = 0;
	/***** Animation 관리 : 종료 *****/

	XMMATRIX m_FinalTransform[BONE_FINAL_TRANSFORM_SIZE];

	bool m_SpecularZero = false; // 스펙큘러를 없애고 싶으면 true
	XMFLOAT4 m_AmbientColor[MATERIAL_SIZE];
	XMFLOAT4 m_DiffuseColor[MATERIAL_SIZE];
	XMFLOAT4 m_SpecularPower[MATERIAL_SIZE];
	XMFLOAT4 m_SpecularColor[MATERIAL_SIZE];
	XMFLOAT3 m_LightDirection = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	unsigned int m_ModelFormat = 0;
	void* m_Model = nullptr;

	HID* m_HID = nullptr; // 포인터를 받아와서 사용하므로 m_HID->Shutdown() 금지
};
