#pragma once

#include "../Shared.h"

class Light
{
public:
	void SetDiffuse(DirectX::XMFLOAT4 diffuse);
	DirectX::XMFLOAT4 GetDiffuse();

protected:
	void Init();
	void Init(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, DirectX::XMFLOAT4 specular);

	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular;
};