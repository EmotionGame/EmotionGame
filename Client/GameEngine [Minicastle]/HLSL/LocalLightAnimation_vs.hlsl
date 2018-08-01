/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
};

cbuffer SkeletonBufferType : register(b2)
{
	matrix boneFinalTransforms[256];
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	uint materialIndex : BLENDINDICES0;
	uint4 boneIndices : BLENDINDICES1;
	float3 boneWeights : BLENDWEIGHTS;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
	uint materialIndex : BLENDINDICES0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LocalLightAnimationVertexShader(VertexInputType input)
{
    PixelInputType output;
    
	// 머터리얼 인덱스 설정
	output.materialIndex = input.materialIndex;

	float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	weights[0] = input.boneWeights.x;
	weights[1] = input.boneWeights.y;
	weights[2] = input.boneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 position = float3(0.0f, 0.0f, 0.0f);
	float3 normal  = float3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 4; i++)
	{
		position += weights[i]*mul(float4(input.position.xyz, 1.0f), boneFinalTransforms[input.boneIndices[i]]).xyz;
		normal   += weights[i]*mul(input.normal,  (float3x3)boneFinalTransforms[input.boneIndices[i]]);
	}

	input.position.xyz = position;
	input.normal = normal;

	// 적절한 행렬 계산을 위해 위치 벡터를 4 단위로 변경합니다.
    input.position.w = 1.0f;

	// 월드, 뷰 및 투영 행렬에 대한 정점의 위치를 ​​계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// 픽셀 쉐이더의 텍스처 좌표를 저장한다.
	output.tex = input.tex;
    
	// 월드 행렬에 대해서만 법선 벡터를 계산합니다.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // 법선 벡터를 정규화합니다.
    output.normal = normalize(output.normal);

    //카메라의 위치와 세계의 정점 위치를 기준으로 보는 방향을 결정
    float4 worldPosition = mul(input.position, worldMatrix);
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

    output.viewDirection = normalize(output.viewDirection);

    return output;
}