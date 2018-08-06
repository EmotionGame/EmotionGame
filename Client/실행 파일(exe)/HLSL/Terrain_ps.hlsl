/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TerrainPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;


	// 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처에서 픽셀 색상을 샘플링합니다.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// 모든 픽셀에 대해 기본 출력 색상을 주변 광원 값으로 설정합니다.
	color = ambientColor;

	// 계산을 위해 빛 방향을 반전시킵니다.
	lightDir = -lightDirection;

	// 이 픽셀의 빛의 양을 계산합니다.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		// 확산 색과 광 강도의 양에 따라 최종 확산 색을 결정합니다.
		color += (diffuseColor * lightIntensity);
	}

	// 최종 빛의 색상을 채웁니다.
	color = saturate(color);

	// 텍스처 픽셀과 최종 밝은 색을 곱하여 결과를 얻습니다.
	color = color * textureColor;

	return color;
}