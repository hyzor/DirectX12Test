#pragma once

#include "HelperFunctions.h"
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;

static const int frameBufferCnt = 3; // Three for triple buffering
static const bool useWarpDevice = false;
static const float rtvClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

class D3dDeviceResources
{
public:
	D3dDeviceResources();
	~D3dDeviceResources();

	// Getters
	ID3D12Device* GetD3dDevice() const { return m_d3dDevice.Get(); }
	IDXGISwapChain3* GetSwapChain() const { return m_swapChain.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_comQueue.Get(); }
	ID3D12DescriptorHeap* GetRtvDescHeap() const { return m_rtvDescriptorHeap.Get(); }
	ID3D12Resource* GetRenderTarget() const { return m_renderTargets[m_frameIdx].Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return m_comAlloc[m_frameIdx].Get(); }
	ID3D12Fence* GetFence() const { return m_fence.Get(); }
	int GetCurFrameIdx() const { return m_frameIdx; }
	D3D12_VIEWPORT GetViewport() const { return m_viewport; }
	D3D12_RECT GetScissorRect() const { return m_scissorRect; }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_frameIdx, m_rtvDesciptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsDescHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void ClearDepthStencilView(ID3D12GraphicsCommandList* comList);
	void ClearRenderTargetView(ID3D12GraphicsCommandList* comList);

	bool CreateDeviceResources(HWND hwnd, UINT windowWidth, UINT windowHeight,
		bool fullscreen, DXGI_SAMPLE_DESC swapChainSampleDesc);
	void WaitForGpu();
	void MoveToNextFrame();
	void Present();
	void IncrementFenceValue() { m_fenceValue[m_frameIdx]++; }

private:
	// D3D12 objects and variables
	Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_comQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[frameBufferCnt];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_comAlloc[frameBufferCnt];
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue[frameBufferCnt];
	int m_frameIdx;
	int m_rtvDesciptorSize;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_dsDescHeap;

	DXGI_FORMAT m_backBufferFormat;
	DXGI_FORMAT m_depthBufferFormat;

	DXGI_SAMPLE_DESC m_swapChainSampleDesc;

	UINT m_windowWidth, m_windowHeight;
	bool m_Fullscreen;
};

