#pragma once

class LocalLight : public AlignedAllocationPolicy<16>
{
public:
	LocalLight();
	LocalLight(const LocalLight& other);
	~LocalLight();

	void SetDirection(float x, float y, float z);
	void SetOpacity(float opacity);
	void SetAmbientColor(float red, float green, float blue, float alpha);
	void SetDiffuseColor(float red, float green, float blue, float alpha);
	void SetSpecularColor(float red, float green, float blue, float alpha);
	void SetEmissiveColor(float red, float green, float blue, float alpha);
	void SetSpecularPower(float power);
	void SetReflection(float reflection);

	XMFLOAT3 GetDirection();
	float GetOpacity();
	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();	
	XMFLOAT4 GetSpecularColor();
	XMFLOAT4 GetEmissiveColor();
	float GetSpecularPower();
	float GetReflection();

private:
	XMFLOAT3 m_direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
	float m_opacity = 1.0f;
	XMFLOAT4 m_ambientColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4 m_diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 m_specularColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	XMFLOAT4 m_emissiveColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	float m_specularPower = 1000.0f;
	float m_reflection = 1.0f;
};