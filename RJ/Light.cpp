////////////////////////////////////////////////////////////////////////////////
// Filename: Light.cpp
////////////////////////////////////////////////////////////////////////////////
#include "DX11_Core.h"
#include "Light.h"


Light::Light()
{
}


Light::Light(const Light& other)
{
}


Light::~Light()
{
}

void Light::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
	return;
}

void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
	return;
}


void Light::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
	return;
}