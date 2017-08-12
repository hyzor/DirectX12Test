#pragma once

#include "Shared.h"
#include "Common.h"

class Texture
{
public:
	Texture();
	Texture(std::vector<UINT8> data);
	Texture(std::wstring name, std::vector<UINT8> data);
	Texture(UINT64 width, UINT height, std::wstring name, std::vector<UINT8> data);
	~Texture();

	void InitD3dResources(
		Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> d3dComList,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> d3dDescHeap,
		CD3DX12_HEAP_PROPERTIES d3dHeap,
		CD3DX12_HEAP_PROPERTIES d3dUplHeap);

private:
	std::vector<UINT8> m_data;
	std::wstring m_name;
	UINT64 m_width;
	UINT m_height;
	UINT m_pixelSizeInBytes;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResourceUplHeap;
};