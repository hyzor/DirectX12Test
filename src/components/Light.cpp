#include "Light.h"

void Light::Init() {
	m_ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_diffuse = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

void Light::Init(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
}

void Light::SetDiffuse(DirectX::XMFLOAT4 diffuse)
{
	m_diffuse = diffuse;
}

DirectX::XMFLOAT4 Light::GetDiffuse()
{
	return m_diffuse;
}
