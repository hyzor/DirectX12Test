#pragma once
#include "Shared.h"

// Macros
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define PI 3.14159265358979323846f
#define degreesToRadians 0.0174532925f

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

struct VertexTex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
};

struct VertexTexNorm {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
	DirectX::XMFLOAT3 norm;
};

enum class LightTypes
{
	DIRECTIONAL = 0,
	POINT = 1,
	SPOT = 2
};

struct PointLightStruct
{
	PointLightStruct(DirectX::XMFLOAT4 pos,
		DirectX::XMFLOAT3 att,
		float range,
		DirectX::XMFLOAT4 ambient,
		DirectX::XMFLOAT4 diffuse,
		DirectX::XMFLOAT4 specular,
		float specularPower)
	{
		this->pos = pos;
		this->att = att;
		this->range = range;
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->specularPower = specularPower;
		this->padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT3 att;
	float range;
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	float specularPower;
	DirectX::XMFLOAT3 padding; // 16 byte boundary
};

struct Material {
	DirectX::XMFLOAT4 emissive;
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	float specularPower;
	float specularIntensity;
	DirectX::XMFLOAT2 padding; // 16 byte boundary
};

static const D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

static const D3D12_INPUT_ELEMENT_DESC inputElementDescTex[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

static const D3D12_INPUT_ELEMENT_DESC inputElementDescTexNorm[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};