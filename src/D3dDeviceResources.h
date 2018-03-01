#pragma once

#include "Shared.h"
#include "Common.h"

using namespace Microsoft::WRL;

static const int frameBufferCnt = 3; // Three for triple buffering
static const bool useWarpDevice = false;
static const float rtvClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

class D3dDeviceResources
{
public:
	D3dDeviceResources();
	~D3dDeviceResources();

	Microsoft::WRL::ComPtr<ID3D12Device> GetD3dDevice() const;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> GetSwapChain() const;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRtvDescHeap() const;
	Microsoft::WRL::ComPtr<ID3D12Resource> GetRenderTarget() const;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const;
	Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() const;

	int GetCurFrameIdx() const;

	D3D12_VIEWPORT GetViewport() const;
	D3D12_RECT GetScissorRect() const;

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

	void ClearDepthStencilView(ID3D12GraphicsCommandList* comList);
	void ClearRenderTargetView(ID3D12GraphicsCommandList* comList);

	bool CreateDeviceResources(HWND hwnd, UINT windowWidth, UINT windowHeight,
		bool fullscreen, DXGI_SAMPLE_DESC swapChainSampleDesc);
	void WaitForGpu();
	void MoveToNextFrame();
	void Present();
	void IncrementFenceValue() { m_fenceValue[m_frameIdx]++; }

	void OnResize(ComPtr<ID3D12GraphicsCommandList> comList, UINT windowWidth, UINT windowHeight);

	void CreateRtvForFrame(CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, UINT frameIdx);
	void CreateDepthStencilBufferAndView(UINT windowWidth, UINT windowHeight);

	D3D12_VIEWPORT CreateViewPort(UINT windowWidth, UINT windowHeight);
	D3D12_RECT CreateScissorRect(UINT windowWidth, UINT windowHeight);

private:
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

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

	DXGI_SAMPLE_DESC m_swapChainSampleDesc;
};

