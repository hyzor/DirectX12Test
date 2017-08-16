#include "Entity.h"

Entity::Entity()
	: Entity(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f))
{
}

Entity::Entity(const XMFLOAT4 worldPos)
{
	Init(worldPos, XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));
}

Entity::Entity(const Entity& entity)
{
	m_worldPos = entity.m_worldPos;
	m_worldMat = entity.m_worldMat;
	m_rotMat = entity.m_rotMat;
	m_meshes = entity.m_meshes;
}

Entity::Entity(const XMFLOAT4 worldPos, std::shared_ptr<Mesh> mesh)
	: Entity(worldPos)
{
	m_meshes.push_back(mesh);
}

Entity::Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale)
{
	Init(worldPos, scale, XMFLOAT3(0.0f, 0.0f, 0.0f));
}

Entity::Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, std::shared_ptr<Mesh> mesh)
	: Entity(worldPos, scale)
{
	m_meshes.push_back(mesh);
}

Entity::Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation)
{
	Init(worldPos, scale, rotation);
}

Entity::Entity(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation, std::shared_ptr<Mesh> mesh)
	: Entity(worldPos, scale, rotation)
{
	m_meshes.push_back(mesh);
}

Entity::~Entity()
{
}

DirectX::XMVECTOR Entity::GetWorldPosVec()
{
	return XMLoadFloat4(&m_worldPos);
}

DirectX::XMFLOAT4 Entity::GetWorldPos()
{
	return m_worldPos;
}

DirectX::XMMATRIX Entity::GetWorldMat()
{
	return XMLoadFloat4x4(&m_worldMat);
}

void Entity::AddMesh(std::shared_ptr<Mesh> mesh)
{
	m_meshes.push_back(mesh);
}

void Entity::Move(const XMFLOAT3 dir)
{
	XMVECTOR newWorldPosVec = XMLoadFloat4(&m_worldPos) + XMLoadFloat3(&dir);
	XMStoreFloat4(&m_worldPos, newWorldPosVec);
	XMStoreFloat4x4(&m_worldMat, XMMatrixTranslationFromVector(newWorldPosVec));
}

void Entity::Rotate(const XMFLOAT3 rotation)
{
	// Rotate
	XMMATRIX rotXMat = XMMatrixRotationX(rotation.x * degreesToRadians);
	XMMATRIX rotYMat = XMMatrixRotationY(rotation.y * degreesToRadians);
	XMMATRIX rotZMat = XMMatrixRotationZ(rotation.z * degreesToRadians);
	XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&m_rotMat, rotMat);

	// Calculate world matrix
	XMMATRIX worldMat = rotMat * XMLoadFloat4x4(&m_worldMat);
	XMStoreFloat4x4(&m_worldMat, worldMat);
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
	D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation)
{
	for (UINT i = 0; i < m_meshes.size(); ++i)
	{
		m_meshes.at(i)->Draw(d3dComList, d3dConstBufferPerObjLocation);
	}
}

boolean const Entity::IsImmovable()
{
	return m_isImmovable;
}

void Entity::SetIsImmovable(boolean isImmovable)
{
	m_isImmovable = isImmovable;
}

void Entity::Init(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation)
{
	// Translate
	m_worldPos = worldPos;
	XMVECTOR worldPosVec = XMLoadFloat4(&worldPos);
	XMMATRIX worldMat = XMMatrixTranslationFromVector(worldPosVec);

	// Scale
	XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMStoreFloat4x4(&m_scaleMat, scaleMat);

	// Rotate
	XMMATRIX rotXMat = XMMatrixRotationX(rotation.x * degreesToRadians);
	XMMATRIX rotYMat = XMMatrixRotationY(rotation.y * degreesToRadians);
	XMMATRIX rotZMat = XMMatrixRotationZ(rotation.z * degreesToRadians);
	XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&m_rotMat, rotMat);

	// Calculate world matrix
	worldMat = scaleMat * rotMat * worldMat;
	XMStoreFloat4x4(&m_worldMat, worldMat);
}
