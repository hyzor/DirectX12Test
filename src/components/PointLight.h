#pragma once

#include "Component.h"
#include "Light.h"
#include "../Common.h"
#include <forward_list>

class PointLight : public Light, public Component {
public:
	PointLight();
	PointLight(float range,
		DirectX::XMFLOAT3 att,
		DirectX::XMFLOAT4 ambient,
		DirectX::XMFLOAT4 diffuse,
		DirectX::XMFLOAT4 specular,
		float specularPower);

	void Update(float dt, float totalTime);
	PointLightStruct GetPointLightStruct(DirectX::XMFLOAT4 pos);

private:
	PointLightStruct m_pointLightStruct;

	float m_range;
	DirectX::XMFLOAT3 m_att;
	float m_specularPower;
};