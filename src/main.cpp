//==================================================================================================
// Author: Jesper Hansson Falkenby
//==================================================================================================
// Code based on:
// http://www.braynzarsoft.net/viewtutorial/q16390-04-directx-12-braynzar-soft-tutorials
// Microsoft DirectX 12 samples at https://github.com/Microsoft/DirectX-Graphics-Samples
//==================================================================================================
// TODO:
//==================================================================================================

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include "AppData.h"
#include "HelperFunctions.h"
#include "Camera.h"

bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
	bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle);
void MainLoop();
bool InitD3d(int wndWidth, int wndHeight, bool wndFullScreen, HWND hwnd, DXGI_SAMPLE_DESC& sampleDesc);
void InitStage(int wndWith, int wndHeight);
void LoadPipelineAssets(DXGI_SAMPLE_DESC& sampleDesc);
void LoadGeometry();
void LoadShaders();
void LoadTextures(UINT64 width, UINT height);

void Cleanup();
void Update();
void UpdatePipeline();
void Render();

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
	LoadPipelineAssets(sampleDesc);

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
	deviceResources = new D3dDeviceResources();

	if (!deviceResources->CreateDeviceResources(hwnd, wndWidth, wndHeight, wndFullScreen, sampleDesc))
		return false;

	return true;
}

void Cleanup()
{
	delete deviceResources;
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
		camDir.x -= speed;
	if ((1 << 16) & keyRight)
		camDir.x += speed;
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

	int curFrameIdx = deviceResources->GetCurFrameIdx();

	// Now copy this new data from CPU to GPU
	memcpy(cbColorMultGpuAddr[curFrameIdx], &cbColorMultData, sizeof(cbColorMultData));

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

	memcpy(cbPerObjGpuAddr[curFrameIdx], &cbPerObject, sizeof(cbPerObject));

	// Apply rotation to cube 2
	rotXMat = XMMatrixRotationX(0.03f);
	rotYMat = XMMatrixRotationY(0.02f);
	rotZMat = XMMatrixRotationZ(0.05f);

	rotMat = rotZMat * (XMLoadFloat4x4(&cube2.rotMat) * (rotXMat * rotYMat));
	XMStoreFloat4x4(&cube2.rotMat, rotMat);

	// Now translate cube 2 and make it smaller
	XMMATRIX translOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2.pos));
	XMMATRIX scaleMat = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMat = scaleMat * translOffsetMat * rotMat * translMat;
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));

	memcpy(cbPerObjGpuAddr[curFrameIdx] + ConstBufferPerObjAlignedSize, &cbPerObject, sizeof(cbPerObject));

	XMStoreFloat4x4(&cube2.worldMat, worldMat);

	// Update light
	rotXMat = XMMatrixRotationX(0.01f);
	rotYMat = XMMatrixRotationY(0.01f);
	rotZMat = XMMatrixRotationZ(0.01f);
	rotMat = rotZMat * pLight1RotMat * (rotXMat * rotYMat);
	pLight1RotMat = rotMat;
	translOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&pointLight.pos));
	worldMat = translOffsetMat * rotMat * translMat;
	XMVECTOR lightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	lightVec = XMVector3TransformCoord(lightVec, worldMat);
	//XMStoreFloat4(&pointLight.pos, lightVec);

	//XMMATRIX translOffsetMatLight = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2.pos));
	//worldMat = translOffsetMatLight * translMat;
	//XMVECTOR lightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVector3TransformCoord(lightVec, worldMat);
	//XMStoreFloat4(&pointLight.pos, lightVec);

	pointLight.diffuse = cbColorMultData.colorMult;

	memcpy(cbPsAddr[curFrameIdx], &pointLight, sizeof(pointLight));

	// Planes
	worldMat = XMLoadFloat4x4(&plane1.worldMat);
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));
	memcpy(cbPerObjGpuAddr[curFrameIdx] + (ConstBufferPerObjAlignedSize * 2), &cbPerObject, sizeof(cbPerObject));
	worldMat = XMLoadFloat4x4(&plane2.worldMat);
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));
	memcpy(cbPerObjGpuAddr[curFrameIdx] + (ConstBufferPerObjAlignedSize * 3), &cbPerObject, sizeof(cbPerObject));
	worldMat = XMLoadFloat4x4(&plane3.worldMat);
	XMStoreFloat4x4(&cbPerObject.world, XMMatrixTranspose(worldMat));
	memcpy(cbPerObjGpuAddr[curFrameIdx] + (ConstBufferPerObjAlignedSize * 4), &cbPerObject, sizeof(cbPerObject));
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

	//ID3D12DescriptorHeap* descHeaps[] = { mainDescHeap[d3dFrameIdx].Get() };
	//d3dComList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	//d3dComList->SetGraphicsRootDescriptorTable(0, mainDescHeap[d3dFrameIdx]->GetGPUDescriptorHandleForHeapStart());

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

	// Draw triangles
	comList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	comList->SetGraphicsRootConstantBufferView(0, constBufUplHeap[curFrameIdx]->GetGPUVirtualAddress());

	comList->IASetVertexBuffers(0, 1, &vertexBufferView);
	comList->IASetIndexBuffer(&indexBufferView);

	comList->SetGraphicsRootConstantBufferView(1, constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress() + (ConstBufferPerObjAlignedSize * 2));
	comList->DrawIndexedInstanced(6, 1, 0, 0, 0); // First quad

	comList->SetGraphicsRootConstantBufferView(1,
			constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress() + (ConstBufferPerObjAlignedSize * 3));
	comList->DrawIndexedInstanced(6, 1, 0, 0, 0); // Second quad

	comList->SetGraphicsRootConstantBufferView(1,
		constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress() + (ConstBufferPerObjAlignedSize * 4));
	comList->DrawIndexedInstanced(6, 1, 0, 0, 0); // Third quad

	// Draw cube 1
	comList->IASetVertexBuffers(0, 1, &cubeVertexBufferView);
	comList->IASetIndexBuffer(&cubeIndexBufferView);
	comList->SetGraphicsRootConstantBufferView(1, constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress());
	comList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

	// Cube 2
	comList->SetGraphicsRootConstantBufferView(1,
		constBufPerObjUplHeap[curFrameIdx]->GetGPUVirtualAddress() + ConstBufferPerObjAlignedSize);
	comList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

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
	ID3D12Device* d3dDevice = deviceResources->GetD3dDevice();

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

	D3D12_ROOT_PARAMETER rootParams[4];

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
		rootParams, // b0 and b1
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
	// Create vertex buffers
	const size_t triangleVertexBufferSize = sizeof(triangleVerticesTexNorm);
	ComPtr<ID3D12Resource> triangleVertexBufferUpl;

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(triangleVertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&triangleVertexBuffer)));
	triangleVertexBuffer->SetName(L"Triangle Vertex Buffer Resource Heap");

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(triangleVertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&triangleVertexBufferUpl)));

	triangleVertexBufferUpl->SetName(L"Triangle Vertex Buffer Upl Resource Heap");

	// Copy data from memory to vertex buffer
	/*UINT8* vertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(triangleVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexDataBegin)));
	memcpy(vertexDataBegin, triangleVerticesTexNorm, triangleVertexBufferSize);
	triangleVertexBuffer->Unmap(0, nullptr);*/

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(triangleVerticesTexNorm);
	vertexData.RowPitch = triangleVertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(comList.Get(), triangleVertexBuffer.Get(), triangleVertexBufferUpl.Get(), 0, 0, 1, &vertexData);
	CD3DX12_RESOURCE_BARRIER triangleVertexResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(triangleVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	comList->ResourceBarrier(1, &triangleVertexResourceBarrier);

	// Cube
	const size_t cubeVertexBufferSize = sizeof(cubeVerticesTexNorm);
	ComPtr<ID3D12Resource> cubeVertexBufferUpl;

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeVertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&cubeVertexBuffer)));

	// Create upload vertex buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeVertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cubeVertexBufferUpl)));
	cubeVertexBufferUpl->SetName(L"Cube Vertex Buffer Upload Resource Heap");
	cubeVertexBuffer->SetName(L"Cube Vertex Buffer Resource Heap");

	D3D12_SUBRESOURCE_DATA cubeVertexData = {};
	cubeVertexData.pData = reinterpret_cast<BYTE*>(cubeVerticesTexNorm);
	cubeVertexData.RowPitch = cubeVertexBufferSize;
	cubeVertexData.SlicePitch = cubeVertexData.RowPitch;

	UpdateSubresources(comList.Get(), cubeVertexBuffer.Get(), cubeVertexBufferUpl.Get(), 0, 0, 1, &cubeVertexData);
	CD3DX12_RESOURCE_BARRIER cubeVertexResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(cubeVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	comList->ResourceBarrier(1, &cubeVertexResourceBarrier);

	//------------------------------------------
	// Create index buffers
	const size_t indexBufferSize = sizeof(DWORD) * triangleIndices.size();
	ComPtr<ID3D12Resource> indexBufferUpl;

	// Create default index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)));
	indexBuffer->SetName(L"Index Buffer Upload Resource Heap");

	// Create upload index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpl)));
	indexBufferUpl->SetName(L"Index Buffer Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(triangleIndices.data());
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(comList.Get(), indexBuffer.Get(), indexBufferUpl.Get(), 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	comList->ResourceBarrier(1, &indexBufferResourceBarrier);

	// Copy data from memory to index buffer
	/*UINT8* indexDataBegin;
	CD3DX12_RANGE readRange2(0, 0);
	ThrowIfFailed(indexBuffer->Map(0, &readRange2, reinterpret_cast<void**>(&indexDataBegin)));
	memcpy(indexDataBegin, triangleIndices.data(), sizeof(DWORD) * triangleIndices.size());
	indexBuffer->Unmap(0, nullptr);*/

	// Cube
	const size_t cubeIndexBufferSize = sizeof(DWORD) * cubeIndices.size();
	ComPtr<ID3D12Resource> cubeIndexBufferUpl;

	// Create upload index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeIndexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cubeIndexBufferUpl)));
	cubeIndexBufferUpl->SetName(L"Cube Index Buffer Resource Heap");

	// Create default index buffer heap
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cubeIndexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cubeIndexBuffer)));
	cubeIndexBuffer->SetName(L"Index Buffer Resource Heap");

	D3D12_SUBRESOURCE_DATA cubeIndexData = {};
	cubeIndexData.pData = reinterpret_cast<BYTE*>(cubeIndices.data());
	cubeIndexData.RowPitch = cubeIndexBufferSize;
	cubeIndexData.SlicePitch = cubeIndexData.RowPitch;

	UpdateSubresources(comList.Get(), cubeIndexBuffer.Get(), cubeIndexBufferUpl.Get(), 0, 0, 1, &cubeIndexData);

	CD3DX12_RESOURCE_BARRIER cubeIndexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(cubeIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	comList->ResourceBarrier(1, &cubeIndexBufferResourceBarrier);

	// Copy data from memory to index buffer
	/*UINT8* cubeIndexDataBegin;
	CD3DX12_RANGE cubeReadRangeIdx(0, 0);
	ThrowIfFailed(cubeIndexBuffer->Map(0, &cubeReadRangeIdx, reinterpret_cast<void**>(&cubeIndexDataBegin)));
	memcpy(cubeIndexDataBegin, cubeIndices.data(), sizeof(DWORD) * cubeIndices.size());
	cubeIndexBuffer->Unmap(0, nullptr);*/

	//------------------------------------------
	// Create constant buffers
	for (UINT i = 0; i < frameBufferCnt; ++i)
	{
		// Create buffer descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 3; // b0, b1 and t0
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
	}

	//------------------------------------------
	// Load textures
	// Create SRV descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap)));

	LoadTextures(checkerboardTexWidth, checkerboardTexHeight); // Only loads a checkerboard texture right now

	//------------------------------------------
	// Close command list and execute it.
	// After this we have our vertex/index buffers on the GPU
	ThrowIfFailed(comList->Close());
	ID3D12CommandList* comLists[] = { comList.Get() };
	deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(comLists), comLists);

	//------------------------------------------
	// Initialize buffer views.
	// Initialize cube vertex buffer view
	cubeVertexBufferView.BufferLocation = cubeVertexBuffer->GetGPUVirtualAddress();
	cubeVertexBufferView.StrideInBytes = sizeof(VertexTexNorm);
	cubeVertexBufferView.SizeInBytes = cubeVertexBufferSize;

	// Initialize cube index buffer view
	cubeIndexBufferView.BufferLocation = cubeIndexBuffer->GetGPUVirtualAddress();
	cubeIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	cubeIndexBufferView.SizeInBytes = cubeIndexBufferSize;

	// Initialize triangle vertex buffer view
	vertexBufferView.BufferLocation = triangleVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(VertexTexNorm);
	vertexBufferView.SizeInBytes = triangleVertexBufferSize;

	// Initialize triangle index buffer view
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	// Wait until assets have been uploaded to the GPU
	deviceResources->IncrementFenceValue();
	deviceResources->WaitForGpu();

	// Command lists are created in the recording state, so close them. Nothing to record yet.
	//comList->Close();
}

void LoadGeometry()
{
	// Simple triangle
	/*triangleVertices[0] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
	triangleVertices[1] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	triangleVertices[2] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) };
	triangleVertices[3] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };*/

	// Triangle
	triangleVerticesTexNorm[0] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, 0.5f) };
	triangleVerticesTexNorm[1] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.5f, -0.5f, 0.5f) };
	triangleVerticesTexNorm[2] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, 0.5f) };
	triangleVerticesTexNorm[3] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) };

	triangleIndices = { 0, 1, 2, 0, 3, 1 };

	//------------------------------------------
	// Textured cube
	// Front face
	cubeVerticesTex[0] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
	cubeVerticesTex[1] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[2] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[3] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };

	// Left side face
	cubeVerticesTex[4] = { XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
	cubeVerticesTex[5] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[6] = { XMFLOAT3(-0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[7] = { XMFLOAT3(-0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };

	// Right side face
	cubeVerticesTex[8] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[9] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };
	cubeVerticesTex[10] = { XMFLOAT3(0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[11] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };

	// Back face
	cubeVerticesTex[12] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
	cubeVerticesTex[13] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[14] = { XMFLOAT3(0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[15] = { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };

	// Top face
	cubeVerticesTex[16] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[17] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };
	cubeVerticesTex[18] = { XMFLOAT3(0.5f, 0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[19] = { XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };

	// Bottom face
	cubeVerticesTex[20] = { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) };
	cubeVerticesTex[21] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) };
	cubeVerticesTex[22] = { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) };
	cubeVerticesTex[23] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) };

	//------------------------------------------
	// Textured cube with normals
	// Front face
	cubeVerticesTexNorm[0] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, 0.5f) };
	cubeVerticesTexNorm[1] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.5f, -0.5f, 0.5f) };
	cubeVerticesTexNorm[2] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, 0.5f) };
	cubeVerticesTexNorm[3] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) };

	// Left side face
	cubeVerticesTexNorm[4] = { XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, -0.5f) };
	cubeVerticesTexNorm[5] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, 0.5f) };
	cubeVerticesTexNorm[6] = { XMFLOAT3(-0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, -0.5f) };
	cubeVerticesTexNorm[7] = { XMFLOAT3(-0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, 0.5f) };

	// Right side face
	cubeVerticesTexNorm[8] = { XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.5f, -0.5f, 0.5f) };
	cubeVerticesTexNorm[9] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, -0.5f) };
	cubeVerticesTexNorm[10] = { XMFLOAT3(0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.5f, -0.5f,  -0.5f) };
	cubeVerticesTexNorm[11] = { XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.5f,  0.5f, 0.5f) };

	// Back face
	cubeVerticesTexNorm[12] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, -0.5f) };
	cubeVerticesTexNorm[13] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, -0.5f) };
	cubeVerticesTexNorm[14] = { XMFLOAT3(0.5f, -0.5f,  -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.5f, -0.5f, -0.5f) };
	cubeVerticesTexNorm[15] = { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, -0.5f) };

	// Top face
	cubeVerticesTexNorm[16] = { XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-0.5f, 0.5f, 0.5f) };
	cubeVerticesTexNorm[17] = { XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, -0.5f) };
	cubeVerticesTexNorm[18] = { XMFLOAT3(0.5f, 0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.5f, 0.5f,  0.5f) };
	cubeVerticesTexNorm[19] = { XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-0.5f, 0.5f, -0.5f) };

	// Bottom face
	cubeVerticesTexNorm[20] = { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.5f, -0.5f, -0.5f) };
	cubeVerticesTexNorm[21] = { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-0.5f, -0.5f, 0.5f) };
	cubeVerticesTexNorm[22] = { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.5f, -0.5f,  0.5f) };
	cubeVerticesTexNorm[23] = { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.5f, -0.5f, -0.5f) };

	cubeIndices = {
		// front face
		0, 1, 2, // first triangle
		0, 3, 1, // second triangle

		// left face
		4, 5, 6, // first triangle
		4, 7, 5, // second triangle

		// right face
		8, 9, 10, // first triangle
		8, 11, 9, // second triangle

		// back face
		12, 13, 14, // first triangle
		12, 15, 13, // second triangle

		// top face
		16, 17, 18, // first triangle
		16, 19, 17, // second triangle

		// bottom face
		20, 21, 22, // first triangle
		20, 23, 21, // second triangle
	};

	numCubeIndices = sizeof(cubeIndices) / sizeof(DWORD);

	//cubeMesh = GenerateCubeTexNorm();
	//sphereMesh = GenerateSphereTexNorm(0.5f, 20, 20);
}

void InitStage(int wndWith, int wndHeight)
{
	// Init camera
	camera = new Camera();

	XMMATRIX tmpMat = XMMatrixPerspectiveFovRH(70.0f*(PI / 180.0f), (float)wndWith / (float)wndHeight, 0.01f, 100.0f);
	camera->SetProjMat(tmpMat);
	XMStoreFloat4x4(&cbPerObject.proj, camera->GetTransposedProjMat());

	camera->SetPos(XMFLOAT4(2.0f, 1.0f, 3.0f, 0.0f));
	camera->SetTarget(XMFLOAT4(0.0f, 0.1f, -2.0f, 0.0f));
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

	// Init point light 1
	//pointLight.pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	pointLight.range = 100.0f;
	pointLight.att = XMFLOAT3(0.2f, 0.3f, 0.2f);
	pointLight.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//XMVECTOR lightVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 lightOffset = XMFLOAT4(-0.1f, 0.0f, 1.5f, 0.0f);
	XMVECTOR lightVec = XMLoadFloat4(&lightOffset) + cube1Vec;
	//lightVec = XMVector3TransformCoord(lightVec, XMLoadFloat4x4(&cube1.worldMat));
	XMStoreFloat4(&pointLight.pos, lightVec);
	/*pointLight.pos.x = XMVectorGetX(lightVec);
	pointLight.pos.y = XMVectorGetY(lightVec);
	pointLight.pos.z = XMVectorGetZ(lightVec);
	pointLight.pos.x -= 0.0f;
	pointLight.pos.y -= 0.5f;
	pointLight.pos.z -= 1.0f;*/
	pLight1RotMat = XMMatrixIdentity();

	plane1.pos = XMFLOAT4(0.0f, -3.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&plane1.rotMat, XMMatrixIdentity());
	XMMATRIX scaleMat = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	XMMATRIX translMat = XMMatrixTranslationFromVector(XMLoadFloat4(&plane1.pos));

	XMMATRIX rotXMat = XMMatrixRotationX(-(90.0f * degreesToRadians));
	XMMATRIX rotYMat = XMMatrixRotationY(0.0f);
	XMMATRIX rotZMat = XMMatrixRotationZ(0.0f);

	// Apply rotation
	XMMATRIX rotMat = XMLoadFloat4x4(&plane1.rotMat) * rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&plane1.rotMat, rotMat);

	XMMATRIX worldMat = scaleMat * rotMat * translMat;
	XMStoreFloat4x4(&plane1.worldMat, worldMat);

	plane2.pos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);
	XMStoreFloat4x4(&plane2.rotMat, XMMatrixIdentity());
	//scaleMat = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	translMat = XMMatrixTranslationFromVector(XMLoadFloat4(&plane2.pos));
	worldMat = scaleMat * translMat;
	XMStoreFloat4x4(&plane2.worldMat, worldMat);

	plane3.pos = XMFLOAT4(-3.0f, 0.0f, -2.0f, 0.0f);
	XMStoreFloat4x4(&plane3.rotMat, XMMatrixIdentity());
	//scaleMat = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	translMat = XMMatrixTranslationFromVector(XMLoadFloat4(&plane3.pos));

	rotXMat = XMMatrixRotationX(0.0f);
	rotYMat = XMMatrixRotationY((90.0f * degreesToRadians));
	rotZMat = XMMatrixRotationZ(0.0f);
	rotMat = rotXMat * rotYMat * rotZMat;

	worldMat = rotMat * scaleMat * translMat;
	XMStoreFloat4x4(&plane3.worldMat, worldMat);
	XMStoreFloat4x4(&plane3.rotMat, rotMat);
}

void LoadTextures(UINT64 width, UINT height)
{
	CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
	ID3D12Device* d3dDevice = deviceResources->GetD3dDevice();

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&checkerboardTexture)));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(checkerboardTexture.Get(), 0, 1)
		+ D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

	// Generate checkerboard texture
	std::vector<UINT8> checkerboardTextureGen = GenerateCheckerboardTexture(
		checkerboardTexWidth, checkerboardTexHeight, checkerboardTexSizeInBytes);

	//UINT64 texUploadBufSize;
	//d3dDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &texUploadBufSize);

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBufUploadHeap)));

	checkerboardTexture->SetName(L"Shader Resource Checkerboard Texture");
	texBufUploadHeap->SetName(L"Shader Texture Upload Resource");
	
	D3D12_SUBRESOURCE_DATA texData = {};
	texData.pData = reinterpret_cast<UINT8*>(checkerboardTextureGen.data());
	texData.RowPitch = checkerboardTexWidth * checkerboardTexSizeInBytes;
	texData.SlicePitch = texData.RowPitch * checkerboardTexHeight;

	UpdateSubresources(comList.Get(), checkerboardTexture.Get(), texBufUploadHeap.Get(), 0, 0, 1, &texData);

	comList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(checkerboardTexture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// Create texture SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	d3dDevice->CreateShaderResourceView(checkerboardTexture.Get(), &srvDesc,
		mainDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart());
}