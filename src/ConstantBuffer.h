#pragma once
#include "IConstantBuffer.h"

template<typename T>
class ConstantBuffer : public IConstantBuffer
{
public:
	ConstantBuffer();
	ConstantBuffer(UINT offset);

	void Init(
		const Microsoft::WRL::ComPtr<ID3D12Device>& d3dDevice,
		const std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>& mainDescHeap,
		const UINT frameBufferCnt);

	const D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAdress(UINT frame);
	const UINT GetOffset();
	UINT8* GetGPUAdress(UINT frame);
	const T& GetBufferData() const;

private:
	T m_bufferData;
};

template<typename T>
ConstantBuffer<T>::ConstantBuffer() {}

template<typename T>
ConstantBuffer<T>::ConstantBuffer(UINT offset)
	: ConstantBuffer()
{
	m_offset = offset;
}

template<typename T>
inline const D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer<T>::GetGPUVirtualAdress(UINT frame)
{
	return m_uplHeap.at(frame)->GetGPUVirtualAddress();
}

template<typename T>
inline UINT8* ConstantBuffer<T>::GetGPUAdress(UINT frame)
{
	return m_gpuAddr.at(frame);
}

template<typename T>
inline const T & ConstantBuffer<T>::GetBufferData() const
{
	return m_bufferData;
}

template<typename T>
inline const UINT ConstantBuffer<T>::GetOffset()
{
	return m_offset;
}

template<typename T>
inline void ConstantBuffer<T>::Init(
	const Microsoft::WRL::ComPtr<ID3D12Device>& d3dDevice,
	const std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>& mainDescHeap,
	const UINT frameBufferCnt)
{
	m_gpuAddr.resize(frameBufferCnt);
	m_uplHeap.resize(frameBufferCnt);

	// Aligned to 256 bytes
	const UINT alignedSize = (sizeof(T) + 255) & ~255;

	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		ThrowIfFailed(d3dDevice.Get()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_uplHeap.at(i).GetAddressOf())
		));

		CD3DX12_CPU_DESCRIPTOR_HANDLE constBufHandle(
			mainDescHeap.at(i)->GetCPUDescriptorHandleForHeapStart(),
			m_offset,
			d3dDevice.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = GetGPUVirtualAdress(i);
		cbvDesc.SizeInBytes = alignedSize;
		d3dDevice->CreateConstantBufferView(&cbvDesc, constBufHandle);

		ZeroMemory(&m_bufferData, sizeof(T));

		// Copy from CPU to GPU
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_uplHeap.at(i)->Map(0, &readRange, reinterpret_cast<void**>(&m_gpuAddr.at(i))));
		memcpy(m_gpuAddr.at(i), &m_bufferData, sizeof(T));
		m_uplHeap.at(i)->Unmap(0, nullptr);
	}
}

template<class T>
const T& IConstantBuffer::GetBufferData() const
{
	return dynamic_cast<const ConstantBuffer<T>&>(*this).GetBufferData();
}