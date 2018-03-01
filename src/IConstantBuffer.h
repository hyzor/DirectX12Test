#pragma once
#include "Shared.h"

class IConstantBuffer 
{
public:
	virtual ~IConstantBuffer() {}

	virtual void Init(
		const Microsoft::WRL::ComPtr<ID3D12Device>& d3dDevice,
		const std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>& mainDescHeap,
		const UINT frameBufferCnt) = 0;

	virtual const D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAdress(UINT frame) = 0;

	template<class T> const T& GetBufferData() const;

	virtual UINT8* GetGPUAdress(UINT frame) = 0;

protected:
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_uplHeap;
	std::vector<UINT8*> m_gpuAddr;

	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvDesc;
	UINT m_offset;
};