//==================================================================================================
// Author: Jesper Hansson Falkenby
//==================================================================================================
// Code based on:
// http://www.braynzarsoft.net/viewtutorial/q16390-04-directx-12-braynzar-soft-tutorials
// Microsoft DirectX 12 samples at https://github.com/Microsoft/DirectX-Graphics-Samples
//==================================================================================================
// TODO:
// - Better resource binding (see: https://digitalerr0r.wordpress.com/2015/09/01/directx-12-programming-4-resources-and-resource-binding/)
//		Have tried--doesn't work, why?
// - Load geometry data from files
// - Add camera
// - Add cubes!
// - Add lights!
//==================================================================================================

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include "common.h"
#include "helpers.h"
#include "Camera.h"

bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
	bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle);
void MainLoop();
bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd, DXGI_SAMPLE_DESC& sampleDesc);
void InitStage(int wndWith, int wndHeight);
void LoadD3dAssets(DXGI_SAMPLE_DESC& sampleDesc);
void LoadGeometry();
void LoadShaders();
void Cleanup();
void Update();
void UpdatePipeline();
void Render();
void WaitForGpu();
void MoveToNextFrame();

// Callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,	UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	//------------------------------------------
	// Initialize Win32 window
	HWND hwnd = NULL;
	LPCTSTR wndName = L"D3d12Test";
	LPCTSTR wndTitle = L"D3d12Test";

	int wndWidth = 1024;
	int wndHeight = 768;
	bool wndFullScreen = false;

	if (!InitWindow(hInstance, hwnd, nCmdShow, wndWidth, wndHeight,
		wndFullScreen, wndName, wndTitle))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	//ZeroMemory(&d3dFenceValue, sizeof(d3dFenceValue));

	// Multi-sampling (none)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	//------------------------------------------
	// Initialize graphics and load assets
	if (!InitD3d(wndWidth, wndHeight, wndFullScreen, hwnd, sampleDesc))
	{
		MessageBox(0, L"Failed to initialize direct3d 12",
			L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// Load shaders, create PSOs and command lists
	LoadShaders();

	// Load geometry
	LoadGeometry();

	// Load assets
	LoadD3dAssets(sampleDesc);

	// Init stage
	InitStage(wndWidth, wndHeight);

	// Start main loop
	MainLoop();

	// Clean up before exiting
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
				appIsRunning = false;
				DestroyWindow(hwnd);
		}

		return 0;

	case WM_DESTROY:
		appIsRunning = false;
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

bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd, DXGI_SAMPLE_DESC& sampleDesc)
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
		ThrowIfFailed(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)));
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

		ThrowIfFailed(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(d3dDevice.GetAddressOf())));
	}

	//------------------------------------------
	// Create D3D RTV command queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // The GPU directly executes this command queue

	ThrowIfFailed(d3dDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(d3dComQueue.GetAddressOf())));

	//------------------------------------------
	// Create D3D swap chain
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = wndWidth;
	backBufferDesc.Height = wndHeight;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCnt;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !wndFullScreen;

	ComPtr<IDXGISwapChain> tmpSwapChain;
	ThrowIfFailed(dxgiFactory->CreateSwapChain(d3dComQueue.Get(), &swapChainDesc, tmpSwapChain.GetAddressOf()));
	d3dSwapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain.Get());
	d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();

	//------------------------------------------
	// Create D3D back buffers (RTVs)
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCnt;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// Heap only used to store output (not visible to shaders)
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&d3dRtvDescriptorHeap)));

	// Get RTV descriptor size in this heap
	d3dRtvDesciptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create RTV and command allocator for each buffer
	for (unsigned int i = 0; i < frameBufferCnt; ++i)
	{
		ThrowIfFailed(d3dSwapChain->GetBuffer(i, IID_PPV_ARGS(d3dRenderTargets[i].GetAddressOf())));

		// Then create RTV that binds the swap chain buffer to the RTV handle
		d3dDevice->CreateRenderTargetView(d3dRenderTargets[i].Get(), nullptr, rtvHandle);

		// Then increment RTV handle by the RTV descriptor size
		rtvHandle.Offset(1, d3dRtvDesciptorSize);

		ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(d3dComAlloc[i].GetAddressOf())));
	}

	//------------------------------------------
	// Create fence and fence event
	ThrowIfFailed(d3dDevice->CreateFence(d3dFenceValue[d3dFrameIdx], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(d3dFence.GetAddressOf())));

	// Create fence event handle
	d3dFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (d3dFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	//------------------------------------------
	// Create depth stencil
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsDescHeap.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc = {};
	dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE dsOptClearValue = {};
	dsOptClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsOptClearValue.DepthStencil.Depth = 1.0f;
	dsOptClearValue.DepthStencil.Stencil = 0;

	d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, wndWidth, wndHeight, 1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsOptClearValue,
		IID_PPV_ARGS(depthStencilBuffer.GetAddressOf()));

	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsDescHeap.GetAddressOf())));
	dsDescHeap->SetName(L"Depth/Stencil Resource Heap");

	d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &dsDesc, dsDescHeap->GetCPUDescriptorHandleForHeapStart());

	//------------------------------------------
	// Specify viewport and scissor rect
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = (float)wndWidth;
	d3dViewport.Height = (float)wndHeight;
	d3dViewport.MinDepth = D3D12_MIN_DEPTH;
	d3dViewport.MaxDepth = D3D12_MAX_DEPTH;

	d3dScissorRect.left = 0;
	d3dScissorRect.top = 0;
	d3dScissorRect.right = wndWidth;
	d3dScissorRect.bottom = wndHeight;

	return true;
}

void Cleanup()
{
	// First wait for GPU to finish
	WaitForGpu();

	CloseHandle(d3dFenceEvent);

	delete camera;

	// Exit swapchain out of fullscreen
	BOOL fs = false;
	if (d3dSwapChain->GetFullscreenState(&fs, NULL))
		d3dSwapChain->SetFullscreenState(false, NULL);
}

void Update()
{
	SHORT keyUp = GetAsyncKeyState(VK_UP);
	SHORT keyDown = GetAsyncKeyState(VK_DOWN);
	SHORT keyLeft = GetAsyncKeyState(VK_LEFT);
	SHORT keyRight = GetAsyncKeyState(VK_RIGHT);
	SHORT keySpace = GetAsyncKeyState(VK_SPACE);
	SHORT keyLeftCtrl = GetAsyncKeyState(VK_CONTROL);
	static const float speed = 0.25f;
	XMFLOAT4 camDir = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	if ((1 << 16) & keyUp)
		camDir.z -= speed;
	if ((1 << 16) & keyDown)
		camDir.z += speed;
	if ((1 << 16) & keyLeft)
		camDir.x += speed;
	if ((1 << 16) & keyRight)
		camDir.x -= speed;
	if ((1 << 16) & keySpace)
		camDir.y -= speed;
	if ((1 << 16) & keyLeftCtrl)
		camDir.y += speed;

	camera->Move(camDir);
	camera->BuildViewMat();
	XMStoreFloat4x4(&cbPerObject.view, camera->GetTransposedViewMat());

	static float rInc = 0.02f;
	static float gInc = 0.04f;
	static float bInc = 0.01f;

	cbColorMultData.colorMult.x += rInc;
	cbColorMultData.colorMult.y += gInc;
	cbColorMultData.colorMult.z += bInc;

	if (cbColorMultData.colorMult.x >= 1.0f || cbColorMultData.colorMult.x <= 0.0f)
	{
		cbColorMultData.colorMult.x = cbColorMultData.colorMult.x >= 1.0f ? 1.0f : 0.0f;
		rInc = -rInc;
	}

	if (cbColorMultData.colorMult.y >= 1.0f || cbColorMultData.colorMult.y <= 0.0f)
	{
		cbColorMultData.colorMult.y = cbColorMultData.colorMult.y >= 1.0f ? 1.0f : 0.0f;
		gInc = -gInc;
	}

	if (cbColorMultData.colorMult.z >= 1.0f || cbColorMultData.colorMult.z <= 0.0f)
	{
		cbColorMultData.colorMult.z = cbColorMultData.colorMult.z >= 1.0f ? 1.0f : 0.0f;
		bInc = -bInc;
	}

	// Now copy this new data from CPU to GPU
	memcpy(cbColorMultGpuAddr[d3dFrameIdx], &cbColorMultData, sizeof(cbColorMultData));

	XMMATRIX rotXMat = XMMatrixRotationX(0.01f);
	XMMATRIX rotYMat = XMMatrixRotationY(0.02f);
	XMMATRIX rotZMat = XMMatrixRotationZ(0.02f);

	// Apply rotation to cube 1
	XMMATRIX rotMat = XMLoadFloat4x4(&cube1.rotMat) * rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&cube1.rotMat, rotMat);

	// Now translate cube 1
	XMMATRIX translMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube1.pos));
	XMMATRIX worldMat = rotMat * translMat;
	XMStoreFloat4x4(&cube1.worldMat, worldMat);

	// Update cube 1 const buffer
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));

	memcpy(cbPerObjGpuAddr[d3dFrameIdx], &cbPerObject, sizeof(cbPerObject));

	// Apply rotation to cube 2
	rotXMat = XMMatrixRotationX(0.03f);
	rotYMat = XMMatrixRotationY(0.02f);
	rotZMat = XMMatrixRotationZ(0.01f);

	rotMat = rotZMat * (XMLoadFloat4x4(&cube2.rotMat) * (rotXMat * rotYMat));
	XMStoreFloat4x4(&cube2.rotMat, rotMat);

	// Now translate cube 2 and make it smaller
	XMMATRIX translOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2.pos));
	XMMATRIX scaleMat = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMat = scaleMat * translOffsetMat * rotMat * translMat;
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));

	memcpy(cbPerObjGpuAddr[d3dFrameIdx] + ConstBufferPerObjAlignedSize, &cbPerObject, sizeof(cbPerObject));

	XMStoreFloat4x4(&cube2.worldMat, worldMat);
}

void UpdatePipeline()
{
	//------------------------------------------
	// Set up pipeline before recording commands
	ThrowIfFailed(d3dComAlloc[d3dFrameIdx]->Reset());

	// Reset command list. When reset we put it into a recording state (recording commands into the command allocator).
	// You may pass an initial pipeline state object as the second parameter. We only clear the RTV.
	ThrowIfFailed(d3dComList->Reset(d3dComAlloc[d3dFrameIdx].Get(), d3dPipelineStateObject.Get()));

	// Set necessary state
	d3dComList->SetGraphicsRootSignature(d3dRootSignature.Get());
	d3dComList->RSSetViewports(1, &d3dViewport);
	d3dComList->RSSetScissorRects(1, &d3dScissorRect);

	ID3D12DescriptorHeap* descHeaps[] = { mainDescHeap[d3dFrameIdx].Get() };
	d3dComList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	//d3dComList->SetGraphicsRootDescriptorTable(0, mainDescHeap[d3dFrameIdx]->GetGPUDescriptorHandleForHeapStart());

	// Indicate that the back buffer will be used as a render target
	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dRenderTargets[d3dFrameIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Get current render target handle so that we can set it as target output
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		d3dFrameIdx, d3dRtvDesciptorSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescHeap->GetCPUDescriptorHandleForHeapStart());

	// Set RTV as target
	d3dComList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//------------------------------------------
	// Start recording commands
	// Clear RTV
	d3dComList->ClearRenderTargetView(rtvHandle, rtvClearColor, 0, nullptr);

	// Clear DSV
	d3dComList->ClearDepthStencilView(dsDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0, nullptr);

	// Draw triangles
	d3dComList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dComList->SetGraphicsRootConstantBufferView(0, constBufUplHeap[d3dFrameIdx]->GetGPUVirtualAddress());

	d3dComList->IASetVertexBuffers(0, 1, &vertexBufferView);
	d3dComList->IASetIndexBuffer(&indexBufferView);

	//d3dComList->SetGraphicsRootConstantBufferView(1, constBufPerObjUplHeap[d3dFrameIdx]->GetGPUVirtualAddress());
	//d3dComList->DrawIndexedInstanced(6, 1, 0, 0, 0); // First quad

	//d3dComList->SetGraphicsRootConstantBufferView(1,
			//constBufPerObjUplHeap[d3dFrameIdx]->GetGPUVirtualAddress() + ConstBufferPerObjAlignedSize);
	//d3dComList->DrawIndexedInstanced(6, 1, 0, 4, 0); // Second quad

	// Draw cube 1
	d3dComList->IASetVertexBuffers(0, 1, &cubeVertexBufferView);
	d3dComList->IASetIndexBuffer(&cubeIndexBufferView);
	d3dComList->SetGraphicsRootConstantBufferView(1, constBufPerObjUplHeap[d3dFrameIdx]->GetGPUVirtualAddress());
	d3dComList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

	// Cube 2
	d3dComList->SetGraphicsRootConstantBufferView(1,
		constBufPerObjUplHeap[d3dFrameIdx]->GetGPUVirtualAddress() + ConstBufferPerObjAlignedSize);
	d3dComList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present
	d3dComList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dRenderTargets[d3dFrameIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(d3dComList->Close());
}

void Render()
{
	UpdatePipeline(); // Start by updating pipeline (sending commands)

	// Create an array of command lists and execute them
	ID3D12CommandList* comLists[] = { d3dComList.Get() };
	d3dComQueue->ExecuteCommandLists(_countof(comLists), comLists);

	// Now present our current backbuffer
	ThrowIfFailed(d3dSwapChain->Present(1, 0)); // V-sync = 1

	MoveToNextFrame();
}

void LoadShaders()
{
	//------------------------------------------
	// Compile shaders
	ComPtr<ID3DBlob> errorBuffer;

	// Compile vertex shader
	ThrowIfFailed(D3DCompileFromFile((shaderPath + vertexShaderStr).c_str(), nullptr, nullptr, "main", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexShader.GetAddressOf(), errorBuffer.GetAddressOf()));

	// Compile pixel shader
	ThrowIfFailed(D3DCompileFromFile((shaderPath + pixelShaderStr).c_str(), nullptr, nullptr, "main", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, pixelShader.GetAddressOf(), errorBuffer.GetAddressOf()));
}

void LoadD3dAssets(DXGI_SAMPLE_DESC& sampleDesc)
{
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	//------------------------------------------
	// Create vertex buffers
	const size_t vertexBufferSize = sizeof(Vertex) * triangleVertices.size();

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)));
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// Copy data from memory to vertex buffer
	UINT8* vertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexDataBegin)));
	memcpy(vertexDataBegin, triangleVertices.data(), sizeof(Vertex) * triangleVertices.size());
	vertexBuffer->Unmap(0, nullptr);

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(triangleVertices.data());
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	// Initialize vertex buffer view
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;

	// Cube
	const size_t cubeVertexBufferSize = sizeof(Vertex) * cubeVertices.size();

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeVertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cubeVertexBuffer)));
	cubeVertexBuffer->SetName(L"Cube Vertex Buffer Resource Heap");

	// Copy data from memory to vertex buffer
	UINT8* cubeVertexDataBegin;
	CD3DX12_RANGE cubeReadRange(0, 0);
	ThrowIfFailed(cubeVertexBuffer->Map(0, &cubeReadRange, reinterpret_cast<void**>(&cubeVertexDataBegin)));
	memcpy(cubeVertexDataBegin, cubeVertices.data(), sizeof(Vertex) * cubeVertices.size());
	cubeVertexBuffer->Unmap(0, nullptr);

	D3D12_SUBRESOURCE_DATA cubeVertexData = {};
	cubeVertexData.pData = reinterpret_cast<BYTE*>(cubeVertices.data());
	cubeVertexData.RowPitch = cubeVertexBufferSize;
	cubeVertexData.SlicePitch = cubeVertexData.RowPitch;

	// Initialize vertex buffer view
	cubeVertexBufferView.BufferLocation = cubeVertexBuffer->GetGPUVirtualAddress();
	cubeVertexBufferView.StrideInBytes = sizeof(Vertex);
	cubeVertexBufferView.SizeInBytes = cubeVertexBufferSize;

	//------------------------------------------
	// Create index buffers
	const size_t indexBufferSize = sizeof(DWORD) * triangleIndices.size();

	// Create upload index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(&
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)));
	indexBuffer->SetName(L"Index Buffer Resource Heap");

	// Copy data from memory to index buffer
	UINT8* indexDataBegin;
	CD3DX12_RANGE readRange2(0, 0);
	ThrowIfFailed(indexBuffer->Map(0, &readRange2, reinterpret_cast<void**>(&indexDataBegin)));
	memcpy(indexDataBegin, triangleIndices.data(), sizeof(DWORD) * triangleIndices.size());
	indexBuffer->Unmap(0, nullptr);

	// Initialize index buffer view
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	// Cube
	const size_t cubeIndexBufferSize = sizeof(DWORD) * cubeIndices.size();

	// Create upload index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeIndexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cubeIndexBuffer)));
	cubeIndexBuffer->SetName(L"Cube Index Buffer Resource Heap");

	// Copy data from memory to index buffer
	UINT8* cubeIndexDataBegin;
	CD3DX12_RANGE cubeReadRangeIdx(0, 0);
	ThrowIfFailed(cubeIndexBuffer->Map(0, &cubeReadRangeIdx, reinterpret_cast<void**>(&cubeIndexDataBegin)));
	memcpy(cubeIndexDataBegin, cubeIndices.data(), sizeof(DWORD) * cubeIndices.size());
	cubeIndexBuffer->Unmap(0, nullptr);

	// Initialize index buffer view
	cubeIndexBufferView.BufferLocation = cubeIndexBuffer->GetGPUVirtualAddress();
	cubeIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	cubeIndexBufferView.SizeInBytes = cubeIndexBufferSize;

	//------------------------------------------
	// Create constant buffers
	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		// Create buffer descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mainDescHeap[i].GetAddressOf())));

		// b0
		// Create resource heap, desc heap and cbv pointer
		ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(constBufUplHeap[i].GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE constBufHandle(mainDescHeap[i]->GetCPUDescriptorHandleForHeapStart(), 0, 0);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constBufUplHeap[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = ConstBufferAlignedSize; // Aligned to 256 bytes
		d3dDevice->CreateConstantBufferView(&cbvDesc, constBufHandle);

		ZeroMemory(&cbColorMultData, sizeof(cbColorMultData));

		// Copy from CPU to GPU
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(constBufUplHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultGpuAddr[i])));
		memcpy(cbColorMultGpuAddr[i], &cbColorMultData, sizeof(cbColorMultData));
		constBufUplHeap[i]->Unmap(0, nullptr);

		// b1
		// Create resource heap, desc heap and cbv pointer
		ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(constBufPerObjUplHeap[i].GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE constBufPerObjHandle(mainDescHeap[i]->GetCPUDescriptorHandleForHeapStart(), 1,
			d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPerObjDesc = {};
		cbvPerObjDesc.BufferLocation = constBufPerObjUplHeap[i]->GetGPUVirtualAddress();
		cbvPerObjDesc.SizeInBytes = ConstBufferAlignedSize; // Aligned to 256 bytes
		d3dDevice->CreateConstantBufferView(&cbvPerObjDesc, constBufPerObjHandle);

		ZeroMemory(&cbPerObject, sizeof(cbPerObject));

		// Copy from CPU to GPU
		CD3DX12_RANGE readRange2(0, 0);
		ThrowIfFailed(constBufPerObjUplHeap[i]->Map(0, &readRange2, reinterpret_cast<void**>(&cbPerObjGpuAddr[i])));
		memcpy(cbPerObjGpuAddr[i], &cbPerObject, sizeof(cbPerObject)); // Cube 1
		memcpy(cbPerObjGpuAddr[i] + ConstBufferPerObjAlignedSize, &cbPerObject, sizeof(cbPerObject)); // Cube 2
		constBufPerObjUplHeap[i]->Unmap(0, nullptr);
	}

	// Wait until assets have been uploaded to the GPU
	d3dFenceValue[d3dFrameIdx]++;
	WaitForGpu();

	//------------------------------------------
	// Create descriptor tables

	D3D12_DESCRIPTOR_RANGE descTableRanges[1];

	// b0
	descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTableRanges[0].NumDescriptors = 2;
	descTableRanges[0].BaseShaderRegister = 0;
	descTableRanges[0].RegisterSpace = 0;
	descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_DESCRIPTOR_TABLE descTable;
	descTable.NumDescriptorRanges = _countof(descTableRanges);
	descTable.pDescriptorRanges = &descTableRanges[0];

	//------------------------------------------
	// Create root signatures
	// b0
	D3D12_ROOT_DESCRIPTOR rootCbvDesc;
	rootCbvDesc.RegisterSpace = 0;
	rootCbvDesc.ShaderRegister = 0;

	// b1
	D3D12_ROOT_DESCRIPTOR rootCbvPerObjDesc;
	rootCbvPerObjDesc.RegisterSpace = 0;
	rootCbvPerObjDesc.ShaderRegister = 1;

	D3D12_ROOT_PARAMETER rootParams[2];

	// b0
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor = rootCbvDesc;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// b1
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[1].Descriptor = rootCbvPerObjDesc;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParams),
		rootParams, 
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

	ComPtr<ID3DBlob> signature;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		signature.GetAddressOf(), nullptr));

	ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(&d3dRootSignature)));

	//------------------------------------------
	// Create pipeline state objects

	// PSO 1
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.pRootSignature = d3dRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&d3dPipelineStateObject)));

	//------------------------------------------
	// Create command lists
	// Command list 1 (for PSO 1)
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3dComAlloc[d3dFrameIdx].Get(),
		d3dPipelineStateObject.Get(), IID_PPV_ARGS(&d3dComList)));

	// Command lists are created in the recording state, so close them. Nothing to record yet.
	d3dComList->Close();
}

void WaitForGpu()
{
	// Schedule a Signal command
	ThrowIfFailed(d3dComQueue->Signal(d3dFence.Get(), d3dFenceValue[d3dFrameIdx]));

	// Wait until fence has processed
	ThrowIfFailed(d3dFence->SetEventOnCompletion(d3dFenceValue[d3dFrameIdx], d3dFenceEvent));
	WaitForSingleObjectEx(d3dFenceEvent, INFINITE, FALSE);

	// Increment fence value for the current frame
	d3dFenceValue[d3dFrameIdx]++;
}

void MoveToNextFrame()
{
	// Schedule a signal command
	const UINT64 curFenceValue = d3dFenceValue[d3dFrameIdx];
	ThrowIfFailed(d3dComQueue->Signal(d3dFence.Get(), curFenceValue));

	// Update frame index
	d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();

	// Wait until next frame is ready
	if (d3dFence->GetCompletedValue() < d3dFenceValue[d3dFrameIdx])
	{
		ThrowIfFailed(d3dFence->SetEventOnCompletion(d3dFenceValue[d3dFrameIdx], d3dFenceEvent));
		WaitForSingleObjectEx(d3dFenceEvent, INFINITE, FALSE);
	}

	// Set fence value for the next frame
	d3dFenceValue[d3dFrameIdx] = curFenceValue + 1;
}

void LoadGeometry()
{
	triangleVertices = {
		Vertex(-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(0.5f,  0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-0.75f, 0.75f, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(0.0f, 0.0f, 0.7f, 1.0f, 0.2f, 1.0f, 1.0f),
		Vertex(-0.75f, 0.0f, 0.7f, 0.1f, 1.0f, 0.5f, 1.0f),
		Vertex(0.0f,  0.75f, 0.7f, 1.0f, 0.0f, 1.0f, 1.0f)
	};

	triangleIndices = { 0, 1, 2, 0, 3, 1 };

	cubeVertices = {
		Vertex(-0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f),

		Vertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f),
	};

	cubeIndices = {
		// Left face
		0, 2, 1,
		1, 2, 3,

		// Right face
		4, 5, 6,
		5, 7, 6,

		// Bottom face
		0, 1, 5,
		0, 5, 4,

		// Top face
		2, 6, 7,
		2, 7, 3,

		// Back face
		0, 4, 6,
		0, 6, 2,

		// Front face
		1, 3, 7,
		1, 7, 5,
	};

	numCubeIndices = sizeof(cubeIndices) / sizeof(DWORD);
}

void InitStage(int wndWith, int wndHeight)
{
	// Init camera
	camera = new Camera();

	XMMATRIX tmpMat = XMMatrixPerspectiveFovRH(70.0f*(PI / 180.0f), (float)wndWith / (float)wndHeight, 0.01f, 100.0f);
	camera->SetProjMat(tmpMat);
	XMStoreFloat4x4(&cbPerObject.proj, camera->GetTransposedProjMat());

	camera->SetPos(XMFLOAT4(0.0f, 0.0f, 5.0f, 0.0f));
	camera->SetTarget(XMFLOAT4(0.0f, -0.1f, 0.0f, 0.0f));
	camera->SetUp(XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f));

	camera->BuildViewMat();
	XMStoreFloat4x4(&cbPerObject.view, camera->GetTransposedViewMat());

	// Init cube 1
	cube1.pos = XMFLOAT4(0.0f, 0.0f, -2.0f, 0.0f);
	XMVECTOR cube1Vec = XMLoadFloat4(&cube1.pos);
	XMStoreFloat4x4(&cube1.rotMat, XMMatrixIdentity());
	XMStoreFloat4x4(&cube1.worldMat, XMMatrixTranslationFromVector(cube1Vec));

	// Init cube 2
	XMFLOAT4 cube2Offset = XMFLOAT4(0.05f, 0.0f, 0.0f, 0.0f);
	XMVECTOR cube2Vec = XMLoadFloat4(&cube2Offset) + cube1Vec;
	XMStoreFloat4(&cube2.pos, cube2Vec);
	XMStoreFloat4x4(&cube2.rotMat, XMMatrixIdentity());
	XMStoreFloat4x4(&cube2.worldMat, XMMatrixTranslationFromVector(cube2Vec));
}