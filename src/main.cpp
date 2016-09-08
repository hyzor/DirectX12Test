#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include "common.h"

bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
	bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle);
void MainLoop();
bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd);
void Cleanup();
void Update();
void UpdatePipeline();
void WaitForPreviousFrame();
void Render();

// Callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,	UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HWND hwnd = NULL;
	LPCTSTR wndName = L"D3d12Test";
	LPCTSTR wndTitle = L"D3d12Test";

	int wndWidth = 1024;
	int wndHeight = 768;
	bool wndFullScreen = false;

	// Initialize and display window
	if (!InitWindow(hInstance, hwnd, nCmdShow, wndWidth, wndHeight,
		wndFullScreen, wndName, wndTitle))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	// initialize direct3d
	if (!InitD3d(wndWidth, wndHeight, wndFullScreen, hwnd))
	{
		MessageBox(0, L"Failed to initialize direct3d 12",
			L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// Start main loop
	MainLoop();

	WaitForPreviousFrame();

	CloseHandle(d3dFenceEvent);

	Cleanup();

	return 0;
}

bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
	bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle)
{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monInf = { sizeof(monInf) };
		GetMonitorInfo(hmon, &monInf);

		width = monInf.rcMonitor.right - monInf.rcMonitor.left;
		height = monInf.rcMonitor.bottom - monInf.rcMonitor.top;
	}

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wndName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(NULL,	wndName, wndTitle, WS_OVERLAPPEDWINDOW,	CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, showWnd);
	UpdateWindow(hwnd);

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd,	UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}

void MainLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (appIsRunning)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// Continue with app code
			Update();
			Render();
		}
	}
}

bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd)
{
	HRESULT hr;

	//------------------------------------------
	// Create D3D device
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		return false;

	IDXGIAdapter1* dxgiAdapter;
	int dxgiAdapterIndex = 0;
	bool dxgiAdapterFound = false;

	while (dxgiFactory->EnumAdapters1(dxgiAdapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND)
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
		hr = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			dxgiAdapterFound = true;
			break;
		}

		dxgiAdapterIndex++;
	}

	// Did we fail to find a compatible D3D12 device?
	if (!dxgiAdapterFound)
		return false;

	hr = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));
	if (FAILED(hr))
		return false;

	//------------------------------------------
	// Create D3D RTV command queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // The GPU directly executes this command queue

	hr = d3dDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&d3dComQueue));
	if (FAILED(hr))
		return false;

	//------------------------------------------
	// Create D3D swap chain
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = wndWidth;
	backBufferDesc.Height = wndHeight;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Multi-sampling (none)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	// Swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCnt;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !wndFullScreen;

	IDXGISwapChain* tmpSwapChain;
	dxgiFactory->CreateSwapChain(d3dComQueue, &swapChainDesc, &tmpSwapChain);
	d3dSwapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain);
	d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();

	//------------------------------------------
	// Create D3D back buffers (RTVs)
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCnt;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// Heap only used to store output (not visible to shaders)
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&d3dRtvDescriptorHeap));
	if (FAILED(hr))
		return false;

	// Get RTV descriptor size in this heap
	d3dRtvDesciptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create RTV for each buffer
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		hr = d3dSwapChain->GetBuffer(i, IID_PPV_ARGS(&d3dRenderTargets[i]));
		if (FAILED(hr))
			return false;

		// Then create RTV that binds the swap chain buffer to the RTV handle
		d3dDevice->CreateRenderTargetView(d3dRenderTargets[i], nullptr, rtvHandle);

		// Then increment RTV handle by the RTV descriptor size
		rtvHandle.Offset(1, d3dRtvDesciptorSize);
	}

	//------------------------------------------
	// Create command allocators
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		hr = d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&d3dComAlloc[i]));
		if (FAILED(hr))
			return false;
	}

	//------------------------------------------
	// Create command list
	hr = d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3dComAlloc[0], NULL, IID_PPV_ARGS(&d3dComList));
	if (FAILED(hr))
		return false;

	d3dComList->Close();

	//------------------------------------------
	// Create fence and fence event
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		hr = d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3dFence[i]));
		if (FAILED(hr))
			return false;

		d3dFenceValue[i] = 0; // Init fence value to 0
	}

	// Create fence event handle
	d3dFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (d3dFenceEvent == nullptr)
		return false;

	//------------------------------------------
	// Create root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr))
		return false;

	hr = d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(&d3dRootSignature));
	if (FAILED(hr))
		return false;

	//------------------------------------------
	// Create shaders
	ID3DBlob* vertexShader;
	ID3DBlob* errorBuffer;

	// Compile vertex shader
	hr = D3DCompileFromFile(L"src/shaders/vertexshader.hlsl", nullptr, nullptr, "main", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
		return false;
	}

	// Get shader bytecode
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

	// Compile pixel shader
	ID3DBlob* pixelShader;
	hr = D3DCompileFromFile(L"src/shaders/pixelshader.hlsl", nullptr, nullptr, "main", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
		return false;
	}

	// Get shader bytecode
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	//------------------------------------------
	// Create input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Create input layout description
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	//------------------------------------------
	// Create pipeline state object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.pRootSignature = d3dRootSignature;
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;

	hr = d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&d3dPipelineStateObject));
	if (FAILED(hr))
		return false;

	//------------------------------------------
	// Create vertex buffer
	Vertex vList[] = {
		{ 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
	};

	int vBufferSize = sizeof(vList);

	// Create default vertex buffer heap
	d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// Create upload heap
	ID3D12Resource* vBufferUploadHeap;
	d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));

	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// Store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList);
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	// Create command to copy data from upload heap to default heap
	UpdateSubresources(d3dComList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// Transition from vertex buffer data from copy dest to vertex buffer state
	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Now exec command list to upload our assets
	d3dComList->Close();
	ID3D12CommandList* commandLists[] = { d3dComList };
	d3dComQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// Increment fence value
	d3dFenceValue[d3dFrameIdx]++;
	hr = d3dComQueue->Signal(d3dFence[d3dFrameIdx], d3dFenceValue[d3dFrameIdx]);
	if (FAILED(hr))
		appIsRunning = false;

	// Create vertex buffer view
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vBufferSize;

	//------------------------------------------
	// Specify viewport and scissor rect
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = wndWidth;
	d3dViewport.Height = wndHeight;
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;

	d3dScissorRect.left= 0;
	d3dScissorRect.top = 0;
	d3dScissorRect.right = wndWidth;
	d3dScissorRect.bottom = wndHeight;

	return true;
}

void Cleanup()
{
	// Wait until all frames are finished before we clean up
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		d3dFrameIdx = i;
		WaitForPreviousFrame();
	}

	// Exit swapchain out of fullscreen
	BOOL fs = false;
	if (d3dSwapChain->GetFullscreenState(&fs, NULL))
		d3dSwapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(d3dDevice);
	SAFE_RELEASE(d3dSwapChain);
	SAFE_RELEASE(d3dComQueue);
	SAFE_RELEASE(d3dRtvDescriptorHeap);
	SAFE_RELEASE(d3dComList);
	SAFE_RELEASE(d3dPipelineStateObject);
	SAFE_RELEASE(d3dRootSignature);
	SAFE_RELEASE(vertexBuffer);

	for (unsigned i = 0; i < frameBufferCnt; ++i)
	{
		SAFE_RELEASE(d3dRenderTargets[i]);
		SAFE_RELEASE(d3dComAlloc[i]);
		SAFE_RELEASE(d3dFence[i]);
	}
}

void Update()
{

}

void UpdatePipeline()
{
	HRESULT hr;

	//------------------------------------------
	// Reset command list
	// Wait for GPU to finish with the command allocator before resetting
	WaitForPreviousFrame();

	hr = d3dComAlloc[d3dFrameIdx]->Reset();
	if (FAILED(hr))
		appIsRunning = false;

	// Reset command list. When reset we put it into a recording state (recording commands into the command allocator).
	// You may pass an initial pipeline state object as the second parameter. We only clear the RTV.
	hr = d3dComList->Reset(d3dComAlloc[d3dFrameIdx], d3dPipelineStateObject);
	if (FAILED(hr))
		appIsRunning = false;

	//------------------------------------------
	// Start recording commands

	// Transition current frame index RTV from the current state to the RTV state
	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dRenderTargets[d3dFrameIdx],
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Get current render target handle so that we can set it as target output
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		d3dFrameIdx, d3dRtvDesciptorSize);

	// Set RTV as target
	d3dComList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Clear RTV
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	d3dComList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Draw triangle
	d3dComList->SetGraphicsRootSignature(d3dRootSignature);
	d3dComList->RSSetViewports(1, &d3dViewport);
	d3dComList->RSSetScissorRects(1, &d3dScissorRect);
	d3dComList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dComList->IASetVertexBuffers(0, 1, &vertexBufferView);
	d3dComList->DrawInstanced(3, 1, 0, 0);

	// Now transition from RTV state to current state
	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dRenderTargets[d3dFrameIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = d3dComList->Close();
	if (FAILED(hr))
		appIsRunning = false;
}

void WaitForPreviousFrame()
{
	HRESULT hr;

	// Swap current RTV buffer index
	d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();

	// GPU has not finished executing command queue if current fence value is less than target fence value
	if (d3dFence[d3dFrameIdx]->GetCompletedValue() < d3dFenceValue[d3dFrameIdx])
	{
		hr = d3dFence[d3dFrameIdx]->SetEventOnCompletion(d3dFenceValue[d3dFrameIdx], d3dFenceEvent);
		if (FAILED(hr))
			appIsRunning = false;

		// Wait until current fence value has reached target value
		WaitForSingleObject(d3dFenceEvent, INFINITE);
	}

	// Increment for next frame
	d3dFenceValue[d3dFrameIdx]++;

	// Swap current RTV buffer index
	//d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();
}

void Render()
{
	HRESULT hr;

	UpdatePipeline(); // Start by updating pipeline (sending commands)

	// Create an array of command lists and execute them
	ID3D12CommandList* comLists[] = { d3dComList };
	d3dComQueue->ExecuteCommandLists(_countof(comLists), comLists);

	// Wait and signal until when we reach the end of our command queue
	hr = d3dComQueue->Signal(d3dFence[d3dFrameIdx], d3dFenceValue[d3dFrameIdx]);
	if (FAILED(hr))
		appIsRunning = false;

	// Now present our current backbuffer
	hr = d3dSwapChain->Present(0, 0);
	if (FAILED(hr))
		appIsRunning = false;
}