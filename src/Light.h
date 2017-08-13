#pragma once

#include "Entity.h"



class Light : public Entity
{
public:
	Light();
	Light(LightTypes type);
	Light(LightTypes type, DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular);
	Light(LightTypes type,
		DirectX::XMFLOAT4 worldPos,
		float range,
		DirectX::XMFLOAT3 att,
		DirectX::XMFLOAT4 ambient,
		DirectX::XMFLOAT4 diffuse,
		DirectX::XMFLOAT4 specular,
		float specularPower);
	~Light();

	void SetDiffuse(DirectX::XMFLOAT4 diffuse);

	const PointLight GetPointLightForShader();

	const LightTypes GetType();

private:
	LightTypes m_type;

	// Pointlight
	float m_range;
	DirectX::XMFLOAT3 m_att;
	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular;
	float m_specularPower;

	// Spotlight

	// Directional light
	DirectX::XMFLOAT3 m_dir;
};