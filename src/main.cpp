#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
#include "d3dx12.h"
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
	hr = d3dComList->Reset(d3dComAlloc[d3dFrameIdx], NULL);
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
	d3dFrameIdx = d3dSwapChain->GetCurrentBackBufferIndex();
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