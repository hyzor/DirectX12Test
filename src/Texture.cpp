#include "Texture.h"

Texture::Texture()
{
	m_name = L"Unknown";
	m_width = 64;
	m_height = 64;
	m_pixelSizeInBytes = 4;
}

Texture::Texture(std::vector<UINT8> data)
	: Texture()
{
	m_data = data;
}

Texture::Texture(std::wstring name, std::vector<UINT8> data)
	: Texture(data)
{
	m_name = name;
}

Texture::Texture(UINT64 width, UINT height, std::wstring name, std::vector<UINT8> data)
	: Texture(name, data)
{
	m_width = width;
	m_height = height;
}

Texture::~Texture()
{

}

void Texture::InitD3dResources(
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> d3dDescHeap,
	CD3DX12_HEAP_PROPERTIES d3dHeap,
	CD3DX12_HEAP_PROPERTIES d3dUplHeap)
{
	CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, m_width, m_height, 1, 1);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dHeap,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_textureResource)));
	m_textureResource->SetName((m_name + L" Texture Resource").c_str());

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textureResource.Get(), 0, 1) 
		+ D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&d3dUplHeap,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_textureResourceUplHeap)));
	m_textureResource->SetName((m_name + L" Texture Resource Upload Heap").c_str());

	D3D12_SUBRESOURCE_DATA texData = {};
	texData.pData = reinterpret_cast<UINT8*>(m_data.data());
	texData.RowPitch = m_width * m_pixelSizeInBytes;
	texData.SlicePitch = texData.RowPitch * m_height;

	UpdateSubresources(d3dComList.Get(), m_textureResource.Get(), m_textureResourceUplHeap.Get(), 0, 0, 1, &texData);

	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	d3dDevice->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, d3dDescHeap.Get()->GetCPUDescriptorHandleForHeapStart());
}