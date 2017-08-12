#pragma once

#include "Common.h"

class Mesh
{
public:
	Mesh();
	Mesh(const std::vector<VertexTexNorm>& vertices, const std::vector<UINT32>& indices);
	Mesh(const std::wstring name, const std::vector<VertexTexNorm>& vertices, const std::vector<UINT32>& indices);
	~Mesh();

	void SetName(const std::wstring name);
	const std::wstring GetName();

	void InitD3dResources(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
		CD3DX12_HEAP_PROPERTIES d3dHeapProps,
		CD3DX12_HEAP_PROPERTIES d3dUplHeapProps);

	void InitD3dViews();

	const D3D12_VERTEX_BUFFER_VIEW GetD3dVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW GetD3dIndexBufferView();

	const size_t GetNumIndices();

	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
		D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation);

private:
	std::vector<VertexTexNorm> m_vertices;
	std::vector<UINT32> m_indices;
	
	// D3d resources
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferUpl;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferUpl;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	size_t m_vertexBufferSize;
	size_t m_indexBufferSize;

	std::wstring m_name;
};