#include "PointLight.h"

PointLight::PointLight()
{
	Light::Init();
	m_range = 100.0f;
	m_att = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_specularPower = 1.0f;
}

PointLight::PointLight(float range, DirectX::XMFLOAT3 att, DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, float specularPower)
{
	Light::Init(ambient, diffuse, specular);
	m_range = range;
	m_att = att;
	m_specularPower = specularPower;
}

void PointLight::Update(float dt, float totalTime)
{
	Component::Update(dt, totalTime);
}

PointLightStruct PointLight::GetPointLightStruct(DirectX::XMFLOAT4 pos)
{
	m_pointLightStruct.ambient = m_ambient;
	m_pointLightStruct.att = m_att;
	m_pointLightStruct.diffuse = m_diffuse;
	m_pointLightStruct.pos = pos;
	m_pointLightStruct.range = m_range;
	m_pointLightStruct.specular = m_specular;
	m_pointLightStruct.specularPower = m_specularPower;
	
	return m_pointLightStruct;
}