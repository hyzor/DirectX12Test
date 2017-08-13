#include "D3dDeviceResources.h"

D3dDeviceResources::D3dDeviceResources() :
	m_frameIdx(0),
	m_viewport(),
	m_rtvDesciptorSize(0),
	m_fenceEvent(0),
	m_fenceValue{}
{
}


D3dDeviceResources::~D3dDeviceResources()
{
	// First wait for GPU to finish
	WaitForGpu();

	CloseHandle(m_fenceEvent);

	// Exit swap chain out of fullscreen
	BOOL fs = false;
	if (m_swapChain->GetFullscreenState(&fs, NULL))
		m_swapChain->SetFullscreenState(false, NULL);
}

void D3dDeviceResources::ClearDepthStencilView(ID3D12GraphicsCommandList* comList)
{
	comList->ClearDepthStencilView(m_dsDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0, nullptr);
}

void D3dDeviceResources::ClearRenderTargetView(ID3D12GraphicsCommandList* comList)
{
	comList->ClearRenderTargetView(GetRenderTargetView(), rtvClearColor, 0, nullptr);
}

bool D3dDeviceResources::CreateDeviceResources(HWND hwnd, UINT windowWidth, UINT windowHeight,
	bool fullscreen, DXGI_SAMPLE_DESC swapChainSampleDesc)
{
	HRESULT hr;

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	//------------------------------------------
	// Create D3D device
	ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

	ComPtr<IDXGIAdapter1> dxgiAdapter;
	int dxgiAdapterIndex = 0;
	bool dxgiAdapterFound = false;

	if (useWarpDevice)
	{
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)));
		ThrowIfFailed(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice)));
	}
	else
	{
		while (dxgiFactory->EnumAdapters1(dxgiAdapterIndex, dxgiAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			dxgiAdapter->GetDesc1(&desc);

			// Skip software devices
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				dxgiAdapterIndex++;
				continue;
			}

			// Now check for D3D12 compatibility
			hr = D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				dxgiAdapterFound = true;
				break;
			}

			dxgiAdapterIndex++;
		}

		// Did we fail to find a compatible D3D12 device?
		if (!dxgiAdapterFound)
		{
			MessageBox(0, L"Failed to find a compatible Direct3D 12 device", L"Error", MB_OK);
			return false;
		}

		ThrowIfFailed(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_d3dDevice.GetAddressOf())));
	}

	//------------------------------------------
	// Create D3D RTV command queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // The GPU directly executes this command queue

	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(m_comQueue.GetAddressOf())));

	//------------------------------------------
	// Create D3D swap chain
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = windowWidth;
	backBufferDesc.Height = windowHeight;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_backBufferFormat = backBufferDesc.Format;

	// Swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCnt;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = swapChainSampleDesc;
	swapChainDesc.Windowed = !fullscreen;

	ComPtr<IDXGISwapChain> tmpSwapChain;
	ThrowIfFailed(dxgiFactory->CreateSwapChain(m_comQueue.Get(), &swapChainDesc, tmpSwapChain.GetAddressOf()));
	m_swapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain.Get());
	m_frameIdx = m_swapChain->GetCurrentBackBufferIndex();

	//------------------------------------------
	// Create D3D back buffers (RTVs)
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCnt;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// Heap only used to store output (not visible to shaders)
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));

	// Get RTV descriptor size in this heap
	m_rtvDesciptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create RTV and command allocator for each buffer
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		CreateRtvForFrame(rtvHandle, i);

		ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_comAlloc[i].GetAddressOf())));
	}

	//------------------------------------------
	// Create fence and fence event
	ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValue[m_frameIdx], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));

	// Create fence event handle
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	//------------------------------------------
	// Create depth stencil
	CreateDepthStencilBufferAndView(windowWidth, windowHeight);

	//------------------------------------------
	// Specify viewport and scissor rect
	m_viewport = CreateViewPort(windowWidth, windowHeight);
	m_scissorRect = CreateScissorRect(windowWidth, windowHeight);

	return true;
}

void D3dDeviceResources::WaitForGpu()
{
	// Schedule a Signal command
	ThrowIfFailed(m_comQueue->Signal(m_fence.Get(), m_fenceValue[m_frameIdx]));

	// Wait until fence has processed
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue[m_frameIdx], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment fence value for the current frame
	m_fenceValue[m_frameIdx]++;
}

void D3dDeviceResources::MoveToNextFrame()
{
	// Schedule a signal command
	const UINT64 curFenceValue = m_fenceValue[m_frameIdx];
	ThrowIfFailed(m_comQueue->Signal(m_fence.Get(), curFenceValue));

	// Update frame index
	m_frameIdx = m_swapChain->GetCurrentBackBufferIndex();

	// Wait until next frame is ready
	if (m_fence->GetCompletedValue() < curFenceValue)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(curFenceValue, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set fence value for the next frame
	m_fenceValue[m_frameIdx] = curFenceValue + 1;
}

void D3dDeviceResources::Present()
{
	// Now present our current back buffer
	ThrowIfFailed(m_swapChain->Present(1, 0)); // V-sync = 1
}

void D3dDeviceResources::OnResize(ComPtr<ID3D12GraphicsCommandList> comList, UINT windowWidth, UINT windowHeight)
{
	// Increment current fence value and wait for the GPU to finish until this point
	IncrementFenceValue();
	WaitForGpu();

	// Reset command list
	ThrowIfFailed(comList->Reset(GetCommandAllocator(), nullptr));

	// Reset render target view buffer resources
	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		m_renderTargets[i].Reset();
	}

	// Reset depth stencil buffer resource
	m_depthStencilBuffer.Reset();

	// Resize swap chain
	ThrowIfFailed(m_swapChain->ResizeBuffers(
		frameBufferCnt,
		windowWidth,
		windowHeight,
		m_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	// Reset current frame index
	m_frameIdx = 0;

	// Create a render target view for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		CreateRtvForFrame(rtvHandle, i);
	}

	// Create depth/stencil buffer and view
	CreateDepthStencilBufferAndView(windowWidth, windowHeight);

	// Transition the resource from its initial state to be used as a depth buffer
	//comList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
		//D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Now execute these resize commands
	ThrowIfFailed(comList->Close());
	ID3D12CommandList* comLists[] = { comList.Get() };
	m_comQueue->ExecuteCommandLists(_countof(comLists), comLists);

	// Increment current fence value and wait for the GPU to finish until this point
	IncrementFenceValue();
	WaitForGpu();

	// Create new viewport
	m_viewport = CreateViewPort(windowWidth, windowHeight);

	// Create new scissor rect
	m_scissorRect = CreateScissorRect(windowWidth, windowHeight);
}

void D3dDeviceResources::CreateRtvForFrame(CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, UINT frameIdx)
{
	ThrowIfFailed(m_swapChain->GetBuffer(frameIdx, IID_PPV_ARGS(m_renderTargets[frameIdx].GetAddressOf())));

	// Then create RTV that binds the swap chain buffer to the RTV handle
	m_d3dDevice->CreateRenderTargetView(m_renderTargets[frameIdx].Get(), nullptr, rtvHandle);

	// Then increment RTV handle by the RTV descriptor size
	rtvHandle.Offset(1, m_rtvDesciptorSize);
}

void D3dDeviceResources::CreateDepthStencilBufferAndView(UINT windowWidth, UINT windowHeight)
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsDescHeap.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc = {};
	dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_depthBufferFormat = dsDesc.Format;

	D3D12_CLEAR_VALUE dsOptClearValue = {};
	dsOptClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsOptClearValue.DepthStencil.Depth = 1.0f;
	dsOptClearValue.DepthStencil.Stencil = 0;

	m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsOptClearValue,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf()));

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsDescHeap.GetAddressOf())));
	m_dsDescHeap->SetName(L"Depth/Stencil Resource Heap");

	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsDesc, m_dsDescHeap->GetCPUDescriptorHandleForHeapStart());
}

D3D12_VIEWPORT D3dDeviceResources::CreateViewPort(UINT windowWidth, UINT windowHeight)
{
	D3D12_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(windowWidth);
	viewport.Height = static_cast<float>(windowHeight);
	viewport.MinDepth = D3D12_MIN_DEPTH;
	viewport.MaxDepth = D3D12_MAX_DEPTH;

	return viewport;
}

D3D12_RECT D3dDeviceResources::CreateScissorRect(UINT windowWidth, UINT windowHeight)
{
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = windowWidth;
	scissorRect.bottom = windowHeight;

	return scissorRect;
}