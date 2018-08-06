/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 SkyDomePixelShader(PixelInputType input) : SV_TARGET
{
	// 스카이 돔에서 픽셀의 높이를 기준으로 정점과 중심을 보정하여 그라디언트 색상을 결정합니다.
	float4 outputColor = shaderTexture.Sample(SampleType, input.tex);

    return outputColor;
}