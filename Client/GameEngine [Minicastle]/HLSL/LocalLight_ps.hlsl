/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor[32];
    float4 diffuseColor[32];
	float4 specularColor[32];
    float4 specularPower[32]; // x만 사용. yzw는 패딩용...
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
    float3 viewDirection : TEXCOORD1;
	uint materialIndex : BLENDINDICES0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 LocalLightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;
	float3 reflection;
	float4 specular;

	// 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처에서 픽셀 색상을 샘플링합니다.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// 모든 픽셀의 기본 출력 색상을 주변 광원 값으로 설정합니다.
	color = ambientColor[input.materialIndex];

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 계산을 위해 빛 방향을 반전시킵니다.
	lightDir = -lightDirection;

	// 이 픽셀의 빛의 양을 계산합니다.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		// 확산 색과 광 강도의 양에 따라 최종 확산 색을 결정합니다.
		color += (diffuseColor[input.materialIndex] * lightIntensity);

		// 최종 빛의 색상을 채 웁니다.
		color = saturate(color);

		/***** Cel shading : 시작 *****/
		/*if (color.x > 0.95f)
			color.x = 1.0f;
		else if (color.x > 0.5f)
			color.x = 0.95f;
		else if (color.x > 0.25f)
			color.x = 0.5f;
		else
			color.x = 0.2f;

		if (color.y > 0.95f)
			color.y = 1.0f;
		else if (color.y > 0.5f)
			color.y = 0.95f;
		else if (color.y > 0.25f)
			color.y = 0.5f;
		else
			color.y = 0.2f;

		if (color.z > 0.95f)
			color.z = 1.0f;
		else if (color.z > 0.5f)
			color.z = 0.95f;
		else if (color.z > 0.25f)
			color.z = 0.5f;
		else
			color.z = 0.25f;*/
		/***** Cel shading : 종료 *****/


		// 빛의 강도, 법선 벡터 및 빛의 방향에 따라 반사 벡터를 계산합니다.
		reflection = normalize(2 * lightIntensity * input.normal - lightDir);

		// 반사 벡터, 시선 방향 및 반사 출력을 기준으로 반사 조명의 양을 결정합니다.


		specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower[input.materialIndex].x);
	}

	// 텍스처 픽셀과 최종 확산 색을 곱하여 최종 픽셀 색상 결과를 얻습니다.
	color = color * textureColor;

	// 스펙큘러 컬러를 적용하기 위해 추가
	float4 sC = saturate(specularColor[input.materialIndex] * specular);

	// 출력 색상의 마지막에 반사 컴포넌트를 추가합니다.
	color = saturate(color + sC);

	//// 확인용
	//if (input.materialIndex == 0)
	//	color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	//if (input.materialIndex == 1)
	//	color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	//if (input.materialIndex == 2)
	//	color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	//if (input.materialIndex == 3)
	//	color = float4(1.0f, 1.0f, 0.0f, 1.0f);
	//if (input.materialIndex == 4)
	//	color = float4(0.0f, 1.0f, 1.0f, 1.0f);
	//if (input.materialIndex == 5)
	//	color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	return color;
}