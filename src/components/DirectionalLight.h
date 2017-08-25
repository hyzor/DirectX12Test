#pragma once

#include "Component.h"
#include "Light.h"

class DirectionalLight : public Light, Component {
public:
	DirectionalLight();
	DirectionalLight(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, DirectX::XMFLOAT3 dir);

	void Update();
private:
	DirectX::XMFLOAT3 m_dir;
};