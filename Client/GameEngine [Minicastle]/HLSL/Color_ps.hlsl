/////////////
// GLOBALS //
/////////////
cbuffer CollisionCheckBuffer : register(b0)
{
	bool collisionCheck;
	bool padding[15];
};

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 color : COLOR;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
	PixelInputType output;

	if (collisionCheck)
	{
		output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		output.color = input.color;
	}

	return output.color;
}