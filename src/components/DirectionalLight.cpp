#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular, DirectX::XMFLOAT3 dir)
{
	Light::Init(ambient, diffuse, specular);
	m_dir = dir;
}

DirectionalLight::DirectionalLight()
{
	Light::Init();
	m_dir = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void DirectionalLight::Update()
{

}
