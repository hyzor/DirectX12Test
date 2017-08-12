#pragma once

#include "Mesh.h"

using namespace DirectX;

class Entity 
{
public:
	Entity();
	Entity(const XMFLOAT4 worldPos);
	Entity(const XMFLOAT4 worldPos, std::shared_ptr<Mesh> mesh);
	Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale);
	Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, std::shared_ptr<Mesh> mesh);
	Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation);
	Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation, std::shared_ptr<Mesh> mesh);
	Entity(const Entity& entity);
	~Entity();

	XMVECTOR GetWorldPosVec();
	XMFLOAT4 GetWorldPos();
	XMMATRIX GetWorldMat();

	void AddMesh(std::shared_ptr<Mesh> mesh);

	void Move(const XMFLOAT4 dir);

	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
		D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation);

protected:
	XMFLOAT4X4 m_worldMat;
	XMFLOAT4X4 m_rotMat;
	XMFLOAT4X4 m_scaleMat;
	XMFLOAT4 m_worldPos;

	std::vector<std::shared_ptr<Mesh>> m_meshes;
};