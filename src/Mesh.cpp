#include "Mesh.h"

Mesh::Mesh()
{

}

Mesh::Mesh(const std::vector<VertexTexNorm>& vertices, const std::vector<UINT32>& indices)
{
	m_vertices = vertices;
	m_indices = indices;
}

Mesh::Mesh(const std::wstring name, const std::vector<VertexTexNorm>& vertices, const std::vector<UINT32>& indices)
	: Mesh(vertices, indices)
{
	m_name = name;
}

Mesh::~Mesh()
{

}

void Mesh::SetName(const std::wstring name)
{
	m_name = name;
}

const std::wstring Mesh::GetName()
{
	return m_name;
}

void Mesh::InitD3dResources(Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
							Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
							CD3DX12_HEAP_PROPERTIES d3dHeapProps,
							CD3DX12_HEAP_PROPERTIES d3dUplHeapProps)
{
	//------------------------------------------
	// Vertex buffers
	m_vertexBufferSize = m_vertices.size() * sizeof(VertexTexNorm);

	// Create vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));
	m_vertexBuffer->SetName((m_name + L" Vertex Buffer Resource Heap").c_str());

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dUplHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBufferUpl)));
	m_vertexBufferUpl->SetName((m_name + L" Vertex Buffer Upload Resource Heap").c_str());

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(m_vertices.data());
	vertexData.RowPitch = m_vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(d3dComList.Get(), m_vertexBuffer.Get(), m_vertexBufferUpl.Get(), 0, 0, 1, &vertexData);

	CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	d3dComList->ResourceBarrier(1, &vertexBufferResourceBarrier);

	//------------------------------------------
	// Index buffers
	m_indexBufferSize = sizeof(UINT32) * m_indices.size();

	// Create index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)));
	m_indexBuffer->SetName((m_name + L" Index Buffer Resource Heap").c_str());

	// Create index buffer upload heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dUplHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBufferUpl)));
	m_indexBufferUpl->SetName((m_name + L" Index Buffer Upload Resource Heap").c_str());

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(m_indices.data());
	indexData.RowPitch = m_indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(d3dComList.Get(), m_indexBuffer.Get(), m_indexBufferUpl.Get(), 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER);

	d3dComList->ResourceBarrier(1, &indexBufferResourceBarrier);
}

void Mesh::InitD3dViews()
{
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(VertexTexNorm);
	m_vertexBufferView.SizeInBytes = static_cast<UINT>(m_vertexBufferSize);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = static_cast<UINT>(m_indexBufferSize);
}

const D3D12_VERTEX_BUFFER_VIEW Mesh::GetD3dVertexBufferView()
{
	return m_vertexBufferView;
}

const D3D12_INDEX_BUFFER_VIEW Mesh::GetD3dIndexBufferView()
{
	return m_indexBufferView;
}

const size_t Mesh::GetNumIndices()
{
	return m_indices.size();
}

void Mesh::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList, D3D12_GPU_VIRTUAL_ADDRESS d3dConstBufferPerObjLocation)
{
	d3dComList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	d3dComList->IASetIndexBuffer(&m_indexBufferView);

	d3dComList->SetGraphicsRootConstantBufferView(1, d3dConstBufferPerObjLocation);
	d3dComList->DrawIndexedInstanced(static_cast<UINT>(m_indices.size()), 1, 0, 0, 0);
}
