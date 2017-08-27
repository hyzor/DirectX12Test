//==================================================================================================
//	Author: Jesper Hansson Falkenby
//==================================================================================================
//	This project is largely based on the following tutorials and samples:
//		http://www.braynzarsoft.net/viewtutorial/q16390-04-directx-12-braynzar-soft-tutorials
//	Microsoft DirectX 12 samples
//		https://github.com/Microsoft/DirectX-Graphics-Samples
//	Sample code for the book "Introduction to 3D Game Programming with DirectX 12"
//		https://github.com/d3dcoder/d3d12book
//==================================================================================================

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers.
#endif

#include "AppData.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "TextureGenerator.h"
#include <windowsx.h>

bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
	bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle);
void MainLoop();
bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd, DXGI_SAMPLE_DESC& sampleDesc);
void InitStage(int wndWith, int wndHeight);
void LoadPipelineAssets(DXGI_SAMPLE_DESC& sampleDesc);
void LoadMeshes();
void LoadShaders();
void LoadTextures();
void OnResize();

// Mouse
void OnMouseMove(WPARAM btnState, int x, int y);
void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);

void Cleanup();
void Update();
void UpdatePipeline();
void Render();

// Callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,	UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	// Init timer
	timer = new Timer();

	//------------------------------------------
	// Initialize Win32 window
	if (!InitWindow(hInstance, hwnd, nCmdShow, wndWidth, wndHeight,
		wndFullScreen, wndName, wndTitle))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

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
	LoadMeshes();

	// Load assets
	LoadPipelineAssets(sampleDesc);

	// Init stage
	InitStage(wndWidth, wndHeight);

	// Update window
	OnResize();

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

	// Window activation/deactivation
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			timer->Stop();
			appIsPaused = true;
		}
		else
		{
			timer->Start();
			appIsPaused = false;
		}

		return 0;

	// Window is resized
	case WM_SIZE:
		wndWidth = LOWORD(lParam);
		wndHeight = HIWORD(lParam);

		// Is d3d device up and running?
		if (deviceResources && deviceResources->GetD3dDevice())
		{
			// Window is minimized
			if (wParam == SIZE_MINIMIZED)
			{
				appIsPaused = true;
				wndIsMaximized = false;
				wndIsMinimized = true;
			}

			// Window is maximized
			else if (wParam == SIZE_MAXIMIZED)
			{
				appIsPaused = false;
				wndIsMaximized = true;
				wndIsMinimized = false;
				OnResize();
			}

			// Window is restored from some state
			else if (wParam == SIZE_RESTORED)
			{
				// Restored from a minimized state
				if (wndIsMinimized)
				{
					appIsPaused = false;
					wndIsMinimized = false;
					OnResize();
				}

				// Restored from a maximized state
				else if (wndIsMaximized)
				{
					appIsPaused = false;
					wndIsMaximized = false;
					OnResize();
				}

				// Is the window being resized currently?
				else if (wndIsResizing)
				{
					// Do nothing
				}
				else
				{
					// Some other call
					OnResize();
				}
			}
		}

		return 0;

	// Window is being resized by dragging
	case WM_ENTERSIZEMOVE:
		timer->Stop();
		appIsPaused = true;
		wndIsResizing = true;
		return 0;

	// Window resize dragging has finished
	case WM_EXITSIZEMOVE:
		timer->Start();
		appIsPaused = false;
		wndIsResizing = false;
		OnResize();
		return 0;

	// Listen for key events
	case WM_KEYDOWN:

		// Press Esc key to exit
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				appIsRunning = false;
				DestroyWindow(hwnd);
		}

		return 0;

	// Window destroy event
	case WM_DESTROY:
		appIsRunning = false;
		PostQuitMessage(0);
		return 0;

	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646349(v=vs.85).aspx
	// Sent when a menu is active and the user presses a key that does not correspond
	// to any mnemonic or accelerator key. This message is sent to the window that owns the menu.
	case WM_MENUCHAR:
		// Do not beep on alt-enter
		return MAKELRESULT(0, MNC_CLOSE);

	// Mouse button(s) pressed
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	// Mouse button(s) released
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	// Mouse is moved
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

	// Reset timer before entering app loop
	timer->Reset();

	while (appIsRunning)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
			timer->Tick();

			// Continue with app code
			if (!appIsPaused)
			{
				Update();
				Render();
			}
		}
	}
}

bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd, DXGI_SAMPLE_DESC& sampleDesc)
{
	deviceResources = new D3dDeviceResources();

	if (!deviceResources->CreateDeviceResources(hwnd, wndWidth, wndHeight, wndFullScreen, sampleDesc))
		return false;

	return true;
}

void Cleanup()
{
	delete deviceResources;
	delete timer;
}

void Update()
{
	const float dt = timer->GetDeltaTime();
	const float totalTime = timer->GetTotalTime();

	SHORT keyUp = GetAsyncKeyState(VK_UP);
	SHORT keyDown = GetAsyncKeyState(VK_DOWN);
	SHORT keyLeft = GetAsyncKeyState(VK_LEFT);
	SHORT keyRight = GetAsyncKeyState(VK_RIGHT);
	SHORT keySpace = GetAsyncKeyState(VK_SPACE);
	SHORT keyLeftCtrl = GetAsyncKeyState(VK_CONTROL);
	SHORT keyA = GetAsyncKeyState(0x41);
	SHORT keyD = GetAsyncKeyState(0x44);
	SHORT keyW = GetAsyncKeyState(0x57);
	SHORT keyS = GetAsyncKeyState(0x53);
	static const float speed = 15.0f;

	if ((1 << 16) & keyW)
		camera->Walk(speed * dt);
	if ((1 << 16) & keyS)
		camera->Walk(-speed * dt);
	if ((1 << 16) & keyA)
		camera->Strafe(-speed * dt);
	if ((1 << 16) & keyD)
		camera->Strafe(speed * dt);

	camera->UpdateViewMat();
	XMStoreFloat4x4(&cbPerObject.view, camera->GetTransposedViewMat());

	static float rInc = 2.0f;
	static float gInc = 4.0f;
	static float bInc = 1.0f;

	cbColorMultData.colorMult.x += rInc * dt;
	cbColorMultData.colorMult.y += gInc * dt;
	cbColorMultData.colorMult.z += bInc * dt;

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

	int curFrameIdx = deviceResources->GetCurFrameIdx();

	// Now copy this new data from CPU to GPU
	memcpy(cbColorMultGpuAddr[curFrameIdx], &cbColorMultData, sizeof(cbColorMultData));

	// Update and set lights
	/*for (std::forward_list<Light>::iterator it = lights.begin(); it != lights.end(); ++it)
	{
		it->SetDiffuse(cbColorMultData.colorMult);

		switch (it->GetType())
		{
			case LightTypes::DIRECTIONAL:
				break;

			case LightTypes::POINT:
				cbPs.pointLight = it->GetPointLightForShader();
				break;

			case LightTypes::SPOT:
				break;
		}
	}*/

	XMStoreFloat4(&cbPs.eyePos, XMLoadFloat4(&camera->GetPos()));

	int index = 0;
	UINT numPointLights = 0;
	for (auto it = entities.begin(); it != entities.end(); ++it, ++index)
	{
		it->Update(dt, totalTime);

		DirectX::XMFLOAT4 worldPos = it->GetWorldPos();

		typeid(PointLight).hash_code();

		for (auto component : it->GetComponentsOfType(typeid(PointLight).hash_code()))
		{
			PointLight* pointLight = static_cast<PointLight*>(component.second);
			cbPs.pointLight[numPointLights] = pointLight->GetPointLightStruct(worldPos);
			numPointLights++;
		}

		// Update current world matrix
		XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(it->GetWorldMat()));

		// Copy per obj constant buffer from CPU to GPU
		memcpy(cbPerObjGpuAddr[curFrameIdx] + (ConstBufferPerObjAlignedSize * (index + 1)), &cbPerObject, sizeof(cbPerObject));
	}

	cbPs.numPointLights = numPointLights;

	// Copy pixel shader constant buffer from CPU to GPU
	memcpy(cbPsAddr[curFrameIdx], &cbPs, sizeof(cbPs));

	// Material
	cbPsMat.mat = mat;
	memcpy(cbPsMatAddr[curFrameIdx], &cbPsMat, sizeof(cbPsMat));
}

void UpdatePipeline()
{
	//------------------------------------------
	// Set up pipeline before recording commands
	ThrowIfFailed(deviceResources->GetCommandAllocator()->Reset());

	// Reset command list. When reset we put it into a recording state (recording commands into the command allocator).
	ThrowIfFailed(comList->Reset(
		deviceResources->GetCommandAllocator(), pipelineStateObject.Get()));

	// Set necessary state
	comList->SetGraphicsRootSignature(rootSignature.Get());
	comList->RSSetViewports(1, &deviceResources->GetViewport());
	comList->RSSetScissorRects(1, &deviceResources->GetScissorRect());

	// Indicate that the back buffer will be used as a render target
	comList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(deviceResources->GetRenderTarget(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Get current render target handle so that we can set it as target output
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = deviceResources->GetRenderTargetView();
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle = deviceResources->GetDepthStencilView();

	// Set RTV as target
	comList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	ID3D12DescriptorHeap* descHeaps[] = { mainDescriptorHeap.Get() };
	comList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	comList->SetGraphicsRootDescriptorTable(2, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//------------------------------------------
	// Start recording commands
	// Clear RTV and DSV
	deviceResources->ClearRenderTargetView(comList.Get());
	deviceResources->ClearDepthStencilView(comList.Get());

	const int curFrameIdx = deviceResources->GetCurFrameIdx();

	// Set light in PS
	comList->SetGraphicsRootConstantBufferView(3, cbPsUplHeap[curFrameIdx]->GetGPUVirtualAddress());

	// Set material in PS
	comList->SetGraphicsRootConstantBufferView(4, cbPsMatUplHeap[curFrameIdx]->GetGPUVirtualAddress());

	comList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	comList->SetGraphicsRootConstantBufferView(0, constBufUplHeap[curFrameIdx]->GetGPUVirtualAddress());

	// Draw entities
	int index = 0;
	for (std::forward_list<Entity>::iterator it = entities.begin(); it != entities.end(); ++it, ++index)
	{
		it->Draw(comList,
			constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress() 
			+ (ConstBufferPerObjAlignedSize * (index + 1)));
	}

	// Indicate that the back buffer will now be used to present
	comList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(deviceResources->GetRenderTarget(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(comList->Close());
}

void Render()
{
	ID3D12CommandQueue* comQueue = deviceResources->GetCommandQueue();

	UpdatePipeline(); // Start by updating pipeline (sending commands)

	// Create an array of command lists and execute them
	ID3D12CommandList* comLists[] = { comList.Get() };
	comQueue->ExecuteCommandLists(_countof(comLists), comLists);

	deviceResources->Present();
	deviceResources->MoveToNextFrame();
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

void LoadPipelineAssets(DXGI_SAMPLE_DESC& sampleDesc)
{
	ID3D12Device* d3dDevice = deviceResources->GetD3dDevice().Get();

	//------------------------------------------
	// Create descriptor tables

	D3D12_DESCRIPTOR_RANGE descTableRanges[1];

	// b0 & b1
	descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTableRanges[0].NumDescriptors = 2;
	descTableRanges[0].BaseShaderRegister = 0;
	descTableRanges[0].RegisterSpace = 0;
	descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descTableRangesSRV[1];

	// t0
	descTableRangesSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTableRangesSRV[0].NumDescriptors = 1;
	descTableRangesSRV[0].BaseShaderRegister = 0;
	descTableRangesSRV[0].RegisterSpace = 0;
	descTableRangesSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_DESCRIPTOR_TABLE descTable;
	descTable.NumDescriptorRanges = _countof(descTableRanges);
	descTable.pDescriptorRanges = &descTableRanges[0];

	// t0
	D3D12_ROOT_DESCRIPTOR_TABLE descTableSRV;
	descTableSRV.NumDescriptorRanges = _countof(descTableRangesSRV);
	descTableSRV.pDescriptorRanges = &descTableRangesSRV[0];

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

	// t0
	D3D12_ROOT_DESCRIPTOR rootTex;
	rootTex.RegisterSpace = 0;
	rootTex.ShaderRegister = 0;

	// b0 - ps
	D3D12_ROOT_DESCRIPTOR rootCbvPsDesc;
	rootCbvPsDesc.RegisterSpace = 0;
	rootCbvPsDesc.ShaderRegister = 0;

	// b1 - ps
	D3D12_ROOT_DESCRIPTOR rootCbvPsMatDesc;
	rootCbvPsMatDesc.RegisterSpace = 0;
	rootCbvPsMatDesc.ShaderRegister = 1;

	D3D12_ROOT_PARAMETER rootParams[5];

	// b0
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor = rootCbvDesc;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// b1
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[1].Descriptor = rootCbvPerObjDesc;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// t0
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[2].DescriptorTable = descTableSRV;
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// b0 - ps
	rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[3].Descriptor = rootCbvPsDesc;
	rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// b1 - ps
	rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[4].Descriptor = rootCbvPsMatDesc;
	rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Sampler s0
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 16;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParams),
		rootParams,
		1, // One static sampler
		&sampler, // s0
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ComPtr<ID3DBlob> signature;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		signature.GetAddressOf(), nullptr));

	ThrowIfFailed(d3dDevice->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	//------------------------------------------
	// Create pipeline state objects

	// PSO 1
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescTexNorm, _countof(inputElementDescTexNorm) };
	psoDesc.pRootSignature = rootSignature.Get();
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

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));

	ID3D12CommandAllocator* comAlloc = deviceResources->GetCommandAllocator();

	//------------------------------------------
	// Create command lists
	// Command list 1 (for PSO 1)
	ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, comAlloc,
		pipelineStateObject.Get(), IID_PPV_ARGS(&comList)));

	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	//------------------------------------------
	// Init meshes
	for (auto it = meshes.begin(); it != meshes.end(); ++it)
	{
		it->second->InitD3dResources(d3dDevice, comList, defaultHeapProp, uploadHeapProp);
	}

	//------------------------------------------
	// Create constant buffers
	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		// Create buffer descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 4; // b0, b1 and t0
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
		memcpy(cbPerObjGpuAddr[i] + ConstBufferPerObjAlignedSize * 2, &cbPerObject, sizeof(cbPerObject)); // Quad 1
		memcpy(cbPerObjGpuAddr[i] + ConstBufferPerObjAlignedSize * 3, &cbPerObject, sizeof(cbPerObject)); // Quad 2
		constBufPerObjUplHeap[i]->Unmap(0, nullptr);

		// Pixel shader b0
		// Create resource heap, desc heap and cbv pointer
		ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(cbPsUplHeap[i].GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE constBufPsHandle(mainDescHeap[i]->GetCPUDescriptorHandleForHeapStart(), 2,
			d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPsDesc = {};
		cbvPsDesc.BufferLocation = cbPsUplHeap[i]->GetGPUVirtualAddress();
		cbvPsDesc.SizeInBytes = ConstBufferPsAlignedSize; // Aligned to 256 bytes
		d3dDevice->CreateConstantBufferView(&cbvPsDesc, constBufPsHandle);

		ZeroMemory(&cbPs, sizeof(cbPs));

		// Copy from CPU to GPU
		CD3DX12_RANGE readRange3(0, 0);
		ThrowIfFailed(cbPsUplHeap[i]->Map(0, &readRange3, reinterpret_cast<void**>(&cbPsAddr[i])));
		memcpy(cbPsAddr[i], &cbPs, sizeof(cbPs));
		cbPsUplHeap[i]->Unmap(0, nullptr);

		// Pixel shader b1
		// Create resource heap, desc heap and cbv pointer
		ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(cbPsMatUplHeap[i].GetAddressOf())));

		CD3DX12_CPU_DESCRIPTOR_HANDLE constBufPsMatHandle(mainDescHeap[i]->GetCPUDescriptorHandleForHeapStart(), 3,
			d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPsMatDesc = {};
		cbvPsMatDesc.BufferLocation = cbPsMatUplHeap[i]->GetGPUVirtualAddress();
		cbvPsMatDesc.SizeInBytes = ConstBufferPsAlignedSize; // Aligned to 256 bytes
		d3dDevice->CreateConstantBufferView(&cbvPsMatDesc, constBufPsMatHandle);

		ZeroMemory(&cbPsMat, sizeof(cbPsMat));

		// Copy from CPU to GPU
		CD3DX12_RANGE readRange4(0, 0);
		ThrowIfFailed(cbPsMatUplHeap[i]->Map(0, &readRange4, reinterpret_cast<void**>(&cbPsMatAddr[i])));
		memcpy(cbPsMatAddr[i], &cbPsMat, sizeof(cbPsMat));
		cbPsMatUplHeap[i]->Unmap(0, nullptr);
	}

	//------------------------------------------
	// Load textures
	// Create SRV descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap)));

	LoadTextures();

	//------------------------------------------
	// Close command list and execute it.
	// After this we have our vertex/index buffers on the GPU
	ThrowIfFailed(comList->Close());
	ID3D12CommandList* comLists[] = { comList.Get() };
	deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(comLists), comLists);

	//------------------------------------------
	// Init vertex and index buffer views
	for (auto it = meshes.begin(); it != meshes.end(); ++it)
	{
		it->second->InitD3dViews();
	}

	// Wait until assets have been uploaded to the GPU
	deviceResources->IncrementFenceValue();
	deviceResources->WaitForGpu();
}

void LoadMeshes()
{
	meshes["Plane"] = GeneratePlaneTexNorm(1.0f, 1.0f);
	meshes["Cube"] = GenerateCubeTexNorm(1.0f, 1.0f, 1.0f);
	meshes["Sphere"] = GenerateSphereTexNorm(0.5f, 20, 20);
}

void InitStage(int wndWidth, int wndHeight)
{
	float dt = timer->GetDeltaTime();

	// Init camera
	camera = new Camera();

	camera->SetPos(XMFLOAT4(2.0f, 1.0f, -5.0f, 1.0f));
	camera->SetTarget(XMFLOAT4(-0.5f, -0.25f, 2.0f, 1.0f));
	camera->SetUp(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

	camera->UpdateViewMat();
	XMStoreFloat4x4(&cbPerObject.view, camera->GetTransposedViewMat());

	// Init cube 1
	Entity cube1(XMFLOAT4(0.0f, 0.0f, 2.0f, 0.0f), meshes["Cube"]);
	entities.push_front(cube1);
	entities.front().AddOnUpdateFunc([](Entity& entity, float dt, float totalTime) { entity.Rotate(XMFLOAT3((90.0f * dt) * -1.0f, 0.0f, 0.0f)); });
	entities.front().AddOnUpdateFunc(
		[](Entity& entity, float dt, float totalTime)
	{
		XMVECTOR newWorldPosVec =
			XMLoadFloat4(&entity.GetWorldSpawnPos()) +
			XMLoadFloat4(&XMFLOAT4(0.0f, sin(totalTime * 3.0f) * 1.0f, 0.0f, 0.0f));

		entity.SetWorldPosVec(newWorldPosVec);
	});
	entities.front().Spawn();

	// Init cube 2
	entities.push_front(Entity(cube1));
	entities.front().Move(XMFLOAT3(1.5f, 0.0f, -1.1f));
	entities.front().AddOnUpdateFunc(
		[](Entity& entity, float dt, float totalTime) 
	{
		XMVECTOR newWorldPosVec = 
			XMLoadFloat4(&entity.GetWorldSpawnPos()) +
			XMLoadFloat4(&XMFLOAT4(cos(totalTime * 3.0f) * 1.0f, sin(totalTime * 3.0f) * 1.0f, 0.0f, 0.0f));

		entity.SetWorldPosVec(newWorldPosVec);
	});

	entities.front().AddOnUpdateFunc([](Entity& entity, float dt, float totalTime) { entity.Rotate(XMFLOAT3(90.0f * dt, 180.0f * dt, 90.0f * dt)); });
	entities.front().Spawn();

	// Init sphere 1
	entities.push_front(Entity(XMFLOAT4(-0.75f, -0.75f, 1.0f, 0.0f), meshes["Sphere"]));
	entities.front().AddOnUpdateFunc([](Entity& entity, float dt, float totalTime) { entity.Rotate(XMFLOAT3(0.0f, 45.0f * dt, 0.0f)); });
	entities.front().Spawn();

	entities.push_front(Entity(cube1.GetWorldPos()));
	entities.front().AddComponent(
		new PointLight(100.0f,
			XMFLOAT3(0.2f, 0.3f, 0.2f),
			XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f),
			XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
			32.0f), typeid(PointLight).hash_code());
	entities.front().Move(XMFLOAT3(0.0f, -0.5f, -2.0f));
	entities.front().Spawn();

	// Init plane 1
	XMFLOAT3 planeScale(3.0f, 3.0f, 3.0f);
	entities.push_front(Entity(XMFLOAT4(0.0f, -1.5f, 2.0f, 1.0f), planeScale, XMFLOAT3(90.0f, 0.0f, 0.0f), meshes["Plane"]));
	entities.front().Spawn();

	// Init plane 2
	entities.push_front(Entity(XMFLOAT4(0.0f, 0.0f, 3.5f, 1.0f), planeScale, XMFLOAT3(0.0f, 0.0f, 0.0f), meshes["Plane"]));
	entities.front().Spawn();

	// Init plane 3
	entities.push_front(Entity(XMFLOAT4(-1.5f, 0.0f, 2.0f, 1.0f), planeScale, XMFLOAT3(0.0f, -(90.0f), 0.0f), meshes["Plane"]));
	entities.front().Spawn();

	// Material
	mat.emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mat.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mat.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mat.specularPower = 32.0f;
	mat.specularIntensity = 1.0f;
}

void LoadTextures()
{
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	textures["Checkerboard"] = std::make_shared<Texture>(checkerboardTexWidth,
		checkerboardTexHeight,
		L"Checkerboard",
		GenerateCheckerboardTexture(checkerboardTexWidth, checkerboardTexHeight, checkerboardTexSizeInBytes));

	textures["Checkerboard"]->InitD3dResources(deviceResources->GetD3dDevice(), comList,
		mainDescriptorHeap, defaultHeapProp, uploadHeapProp);
}

void OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - lastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - lastMousePos.y));

		camera->Pitch(dy);
		camera->Yaw(dx);
	}

	lastMousePos = XMFLOAT2(static_cast<float>(x), static_cast<float>(y));
}

void OnMouseDown(WPARAM btnState, int x, int y)
{
	lastMousePos = XMFLOAT2(static_cast<float>(x), static_cast<float>(y));

	SetCapture(hwnd);
}

void OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void OnResize()
{
	assert(deviceResources);
	assert(deviceResources->GetD3dDevice());
	assert(deviceResources->GetSwapChain());
	assert(deviceResources->GetCommandAllocator());

	deviceResources->OnResize(comList, wndWidth, wndHeight);

	camera->SetLens(60.0f * (PI / 180.0f), (float)wndWidth / (float)wndHeight, 0.01f, 1000.0f);
	XMStoreFloat4x4(&cbPerObject.proj, camera->GetTransposedProjMat());
}