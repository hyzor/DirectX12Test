#pragma once

#include "Mesh.h"
#include "components/Component.h"
#include <unordered_map>
#include <forward_list>
#include <utility>

using namespace DirectX;

class Entity 
{
public:
	typedef void(*OnUpdateFunc)(Entity& entity, float dt, float totalTime);

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

	XMFLOAT4 GetWorldSpawnPos();

	void SetWorldPos(XMFLOAT3 worldPos);
	void SetWorldPosVec(XMVECTOR worldPosVec);

	void AddMesh(std::shared_ptr<Mesh> mesh);

	void Move(const XMFLOAT3 dir);

	void Rotate(const XMFLOAT3 rotation);

	void Activate();
	void Deactivate();
	void Spawn();
	void Respawn();

	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
		D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation);

	void AddComponent(Component* component, size_t nameHash);
	void AddComponent(Component* component, size_t nameHash, UINT id);
	std::forward_list<std::pair<UINT, Component*>> GetComponentsOfType(size_t nameHash);
	Component* GetComponent(size_t nameHash, UINT id);

	void Update(float dt, float totalTime);
	void AddOnUpdateFunc(OnUpdateFunc onUpdateFunc);

protected:
	XMFLOAT4X4 m_worldMat;
	XMFLOAT4X4 m_rotMat;
	XMFLOAT4X4 m_scaleMat;
	XMFLOAT4X4 m_transMat;

	XMFLOAT4 m_worldPos;
	XMFLOAT3 m_worldScale;

	XMFLOAT4 m_spawnWorldPos;
	XMFLOAT4X4 m_spawnRotMat;
	XMFLOAT4X4 m_spawnScaleMat;

	bool m_worldIsDirty = true;
	bool m_isSpawned = false;
	bool m_isActive = false;

	std::vector<std::shared_ptr<Mesh>> m_meshes;

	//std::unordered_map<std::string, std::forward_list<Component*>> m_components;
	std::unordered_map<size_t, std::forward_list<std::pair<UINT, Component*>>> m_components;
	UINT m_componentsAdded = 0;
	std::forward_list<OnUpdateFunc> m_onUpdateFuncList;

private:
	void Init(const XMFLOAT4 worldPos, const XMFLOAT3 scale, const XMFLOAT3 rotation);
};