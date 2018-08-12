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
	float4 outputColor = shaderTexture.Sample(SampleType, input.tex);

    return outputColor;
}