#include "stdafx.h"
#include "LocalLight.h"


LocalLight::LocalLight()
{
}


LocalLight::LocalLight(const LocalLight& other)
{
}


LocalLight::~LocalLight()
{
}

void LocalLight::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
}

void LocalLight::SetOpacity(float opacity)
{
	m_opacity = opacity;
}

void LocalLight::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
	return;
}

void LocalLight::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}

void LocalLight::SetSpecularColor(float red, float green, float blue, float alpha)
{
	m_specularColor = XMFLOAT4(red, green, blue, alpha);
}

void LocalLight::SetEmissiveColor(float red, float green, float blue, float alpha)
{
	m_emissiveColor = XMFLOAT4(red, green, blue, alpha);
}

void LocalLight::SetSpecularPower(float power)
{
	m_specularPower = power;
}

void LocalLight::SetReflection(float reflection)
{
	m_reflection = reflection;
}


XMFLOAT3 LocalLight::GetDirection()
{
	return m_direction;
}

float LocalLight::GetOpacity()
{
	return m_opacity;
}

XMFLOAT4 LocalLight::GetAmbientColor()
{
	return m_ambientColor;
}

XMFLOAT4 LocalLight::GetDiffuseColor()
{
	return m_diffuseColor;
}

XMFLOAT4 LocalLight::GetSpecularColor()
{
	return m_specularColor;
}

XMFLOAT4 LocalLight::GetEmissiveColor()
{
	return m_emissiveColor;
}

float LocalLight::GetSpecularPower()
{
	return m_specularPower;
}

float LocalLight::GetReflection()
{
	return m_reflection;
}