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
    float4 specularPower[32]; // x�� ���. yzw�� �е���...
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

	// �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� �ؽ�ó���� �ȼ� ������ ���ø��մϴ�.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// ��� �ȼ��� �⺻ ��� ������ �ֺ� ���� ������ �����մϴ�.
	color = ambientColor[input.materialIndex];

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ����� ���� �� ������ ������ŵ�ϴ�.
	lightDir = -lightDirection;

	// �� �ȼ��� ���� ���� ����մϴ�.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		// Ȯ�� ���� �� ������ �翡 ���� ���� Ȯ�� ���� �����մϴ�.
		color += (diffuseColor[input.materialIndex] * lightIntensity);

		// ���� ���� ������ ä ��ϴ�.
		color = saturate(color);

		/***** Cel shading : ���� *****/
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
		/***** Cel shading : ���� *****/


		// ���� ����, ���� ���� �� ���� ���⿡ ���� �ݻ� ���͸� ����մϴ�.
		reflection = normalize(2 * lightIntensity * input.normal - lightDir);

		// �ݻ� ����, �ü� ���� �� �ݻ� ����� �������� �ݻ� ������ ���� �����մϴ�.


		specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower[input.materialIndex].x);
	}

	// �ؽ�ó �ȼ��� ���� Ȯ�� ���� ���Ͽ� ���� �ȼ� ���� ����� ����ϴ�.
	color = color * textureColor;

	// ����ŧ�� �÷��� �����ϱ� ���� �߰�
	float4 sC = saturate(specularColor[input.materialIndex] * specular);

	// ��� ������ �������� �ݻ� ������Ʈ�� �߰��մϴ�.
	color = saturate(color + sC);

	//// Ȯ�ο�
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