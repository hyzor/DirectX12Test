#include "Light.h"

Light::Light()
	: Entity()
{
	m_type = LightTypes::DIRECTIONAL;
	m_dir = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_worldPos = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

Light::Light(LightTypes type)
	: Light()
{
	m_type = type;
}

Light::Light(LightTypes type, DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular)
	: Light(type)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
}

Light::Light(LightTypes type,
	DirectX::XMFLOAT4 worldPos,
	float range,
	DirectX::XMFLOAT3 att,
	DirectX::XMFLOAT4 ambient,
	DirectX::XMFLOAT4 diffuse,
	DirectX::XMFLOAT4 specular,
	float specularPower)
	: Light(type, ambient, diffuse, specular)
{
	m_worldPos = worldPos;
	m_att = att;
	m_range = range;
	m_specularPower = specularPower;
}

Light::~Light()
{

}

void Light::SetDiffuse(DirectX::XMFLOAT4 diffuse)
{
	m_diffuse = diffuse;
}

const PointLight Light::GetShaderPreparedPointLight()
{
	return PointLight(m_worldPos, m_att, m_range, m_ambient, m_diffuse, m_specular, m_specularPower);
}

const LightTypes Light::GetType()
{
	return m_type;
}
