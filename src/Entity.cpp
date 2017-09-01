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
	m_scaleMat = entity.m_scaleMat;
	m_meshes = entity.m_meshes;
	m_transMat = entity.m_transMat;
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
	for (auto it : m_components)
	{
		for (auto it2 : it.second)
		{
			delete it2.second;
		}
	}

	m_components.clear();
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

DirectX::XMFLOAT4 Entity::GetWorldSpawnPos()
{
	return m_spawnWorldPos;
}

void Entity::SetWorldPos(XMFLOAT3 worldPos)
{
	XMVECTOR newWorldPosVec = XMLoadFloat4(&XMFLOAT4(worldPos.x, worldPos.y, worldPos.z, 1.0f));
	XMStoreFloat4x4(&m_transMat, XMMatrixTranslationFromVector(newWorldPosVec));
	m_worldIsDirty = true;

	//XMStoreFloat4(&m_worldPos, newWorldPosVec);
	//XMStoreFloat4x4(&m_worldMat, XMMatrixTranslationFromVector(newWorldPosVec));
}

void Entity::SetWorldPosVec(XMVECTOR worldPosVec)
{
	XMStoreFloat4(&m_worldPos, worldPosVec);
	XMStoreFloat4x4(&m_transMat, XMMatrixTranslationFromVector(worldPosVec));
	m_worldIsDirty = true;

	//XMMATRIX worldMat = XMMatrixTranslationFromVector(worldPosVec);
	//worldMat = XMLoadFloat4x4(&m_scaleMat) * XMLoadFloat4x4(&m_rotMat) * worldMat;
	//XMStoreFloat4x4(&m_worldMat, worldMat);
}

void Entity::AddMesh(std::shared_ptr<Mesh> mesh)
{
	m_meshes.push_back(mesh);
}

void Entity::Move(const XMFLOAT3 dir)
{
	XMVECTOR newWorldPosVec = XMLoadFloat4(&m_worldPos) + XMLoadFloat3(&dir);
	XMStoreFloat4(&m_worldPos, newWorldPosVec);
	XMStoreFloat4x4(&m_transMat, XMMatrixTranslationFromVector(newWorldPosVec));

	m_worldIsDirty = true;

	//XMStoreFloat4x4(&m_worldMat, XMMatrixTranslationFromVector(newWorldPosVec));
}

void Entity::Rotate(const XMFLOAT3 rotation)
{
	// Rotate
	XMMATRIX rotXMat = XMMatrixRotationX(rotation.x * degreesToRadians);
	XMMATRIX rotYMat = XMMatrixRotationY(rotation.y * degreesToRadians);
	XMMATRIX rotZMat = XMMatrixRotationZ(rotation.z * degreesToRadians);
	XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&m_rotMat, XMLoadFloat4x4(&m_rotMat) * rotMat);

	m_worldIsDirty = true;
	// Calculate world matrix
	//XMMATRIX worldMat = rotMat * XMLoadFloat4x4(&m_worldMat);
	//XMStoreFloat4x4(&m_worldMat, worldMat);
}

void Entity::Activate()
{
	m_isActive = true;
}

void Entity::Deactivate()
{
	m_isActive = false;
}

void Entity::Spawn()
{
	if (!m_isSpawned)
	{
		m_spawnWorldPos = m_worldPos;
		m_spawnRotMat = m_rotMat;
		m_spawnScaleMat = m_scaleMat;

		m_isSpawned = true;
		m_isActive = true;
	}
}

void Entity::Respawn()
{
	m_isActive = false;
	m_isSpawned = false;

	XMVECTOR worldPosVec = XMLoadFloat4(&XMFLOAT4(m_spawnWorldPos.x, m_spawnWorldPos.y, m_spawnWorldPos.z, 1.0f));
	XMMATRIX worldMat = XMMatrixTranslationFromVector(worldPosVec);
	m_rotMat = m_spawnRotMat;
	m_scaleMat = m_spawnScaleMat;

	// Calculate world matrix
	worldMat = XMLoadFloat4x4(&m_scaleMat) * XMLoadFloat4x4(&m_rotMat) * worldMat;
	XMStoreFloat4x4(&m_worldMat, worldMat);

	m_isActive = true;
	m_isSpawned = true;
}

bool Entity::IsActive()
{
	return m_isActive;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
	D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation)
{
	if (m_isSpawned && m_isActive)
	{
		for (UINT i = 0; i < m_meshes.size(); ++i)
		{
			m_meshes.at(i)->Draw(d3dComList, d3dConstBufferPerObjLocation);
		}
	}
}

void Entity::AddComponent(Component* component, size_t nameHash)
{
	AddComponent(component, nameHash, m_componentsAdded);
}

void Entity::AddComponent(Component* component, size_t nameHash, UINT id)
{
	m_components[nameHash].push_front(std::make_pair(id, component));
}

std::forward_list<std::pair<UINT, Component*>> Entity::GetComponentsOfType(size_t nameHash)
{
	return m_components[nameHash];
}

Component* Entity::GetComponent(size_t nameHash, UINT id)
{
	for (auto it : m_components[nameHash])
	{
		if (it.first == id)
		{
			return it.second;
		}
	}

	return nullptr;
}

void Entity::Update(float dt, float totalTime)
{
	if (m_isSpawned && m_isActive)
	{
		for (auto it : m_components)
		{
			for (auto componentPair : it.second)
			{
				componentPair.second->Update(dt, totalTime);
			}
		}

		for (auto onUpdateFunc : m_onUpdateFuncList)
		{
			onUpdateFunc(*this, dt, totalTime);
		}

		if (m_worldIsDirty)
		{
			XMMATRIX worldMat = XMLoadFloat4x4(&m_scaleMat) * XMLoadFloat4x4(&m_rotMat) * XMLoadFloat4x4(&m_transMat);
			XMStoreFloat4x4(&m_worldMat, worldMat);
			m_worldIsDirty = false;
		}
	}
}

void Entity::AddOnUpdateFunc(OnUpdateFunc onUpdateFunc)
{
	m_onUpdateFuncList.push_front(onUpdateFunc);
}

void Entity::Init(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation)
{
	// Translate
	m_worldPos = worldPos;
	XMVECTOR worldPosVec = XMLoadFloat4(&worldPos);
	XMMATRIX transMat = XMMatrixTranslationFromVector(worldPosVec);
	XMStoreFloat4x4(&m_transMat, transMat);

	// Scale
	m_worldScale = scale;
	XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMStoreFloat4x4(&m_scaleMat, scaleMat);

	// Rotate
	XMMATRIX rotXMat = XMMatrixRotationX(rotation.x * degreesToRadians);
	XMMATRIX rotYMat = XMMatrixRotationY(rotation.y * degreesToRadians);
	XMMATRIX rotZMat = XMMatrixRotationZ(rotation.z * degreesToRadians);
	XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&m_rotMat, rotMat);

	// Calculate world matrix
	//XMStoreFloat4x4(&m_worldMat, scaleMat * rotMat * transMat);
	m_worldIsDirty = true;

	m_spawnWorldPos = m_worldPos;
	m_spawnRotMat = m_rotMat;
	m_spawnScaleMat = m_scaleMat;
}