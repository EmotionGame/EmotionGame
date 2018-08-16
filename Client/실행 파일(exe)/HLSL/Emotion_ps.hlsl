/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);

cbuffer EmotionBuffer : register(b0)
{
	int maxEmotionValue;
	int maxEmotionKind;
	int screenWidth;
	int screenHeight;
};

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
float4 EmotionPixelShader(PixelInputType input) : SV_TARGET
{
	float4 output = shaderTexture.Sample(SampleType, input.tex);

	/*float posX = input.position.x / (screenWidth / 2.0f) - 1.0f;
	float posY = input.position.y / (screenHeight / 2.0f) - 1.0f;
	float radius = maxEmotionValue / 100.0f;*/
	if (maxEmotionValue != 5)
	{
		float posX = input.position.x - ((float)screenWidth / 2.0f);
		float posY = input.position.y - ((float)screenHeight / 2.0f);
		float radius = ((float)maxEmotionValue / 100.0f) * (screenWidth / 2.0f);

		if (maxEmotionValue >= 90)
		{
			output.w = 0.0f;
			if (maxEmotionKind == 0)
			{
				output.x = 0.1f;
				output.y = 0.1f;
				output.z = 0.1f;
			}
			else if (maxEmotionKind == 1)
			{
				output.x = 0.1f;
			}
			else if (maxEmotionKind == 2)
			{
				output.x = 0.0f;
				output.y = 0.0f;
				output.z = 0.0f;
				output.w = 0.2f;
			}
			else
			{
				output.x = 0.1f;
				output.y = 0.1f;
			}

		}
		else if (((posX * posX) + (posY * posY)) <= (radius * radius))
		{
			output.w = 0.0f;
			if (maxEmotionKind == 0)
			{
				output.x = 0.1f;
				output.y = 0.1f;
				output.z = 0.1f;
			}
			else if (maxEmotionKind == 1)
			{
				output.x = 0.1f;
			}
			else if (maxEmotionKind == 2)
			{
				output.x = 0.0f;
				output.y = 0.0f;
				output.z = 0.0f;
				output.w = 0.2f;
			}
			else
			{
				output.x = 0.1f;
				output.y = 0.1f;
			}
		}
	}

	return output;
}