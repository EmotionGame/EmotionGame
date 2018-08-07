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
float4 DelayLoadingPixelShader(PixelInputType input) : SV_TARGET
{
	float4 output = shaderTexture.Sample(SampleType, input.tex);

	return output;
}