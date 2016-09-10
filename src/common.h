#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <array>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;

// Macros
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

// D3D12 objects and variables
const int frameBufferCnt = 3; // Three for triple buffering
ComPtr<ID3D12Device> d3dDevice;
ComPtr<IDXGISwapChain3> d3dSwapChain;
ComPtr<ID3D12CommandQueue> d3dComQueue;
ComPtr<ID3D12DescriptorHeap> d3dRtvDescriptorHeap;
ComPtr<ID3D12Resource> d3dRenderTargets[frameBufferCnt];
ComPtr<ID3D12CommandAllocator> d3dComAlloc[frameBufferCnt];
ComPtr<ID3D12GraphicsCommandList> d3dComList;
ComPtr<ID3D12Fence> d3dFence;
ComPtr<ID3D12RootSignature> d3dRootSignature;
ComPtr<ID3D12PipelineState> d3dPipelineStateObject;
HANDLE d3dFenceEvent;
UINT64 d3dFenceValue[frameBufferCnt];
int d3dFrameIdx;
int d3dRtvDesciptorSize;

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

D3D12_VIEWPORT d3dViewport;
D3D12_RECT d3dScissorRect;

const std::wstring shaderPath = L"src/shaders/";
const LPCWSTR vertexShaderStr = L"vertexshader.hlsl";
const LPCWSTR pixelShaderStr = L"pixelshader.hlsl";

/*
int constBufferPerObjectAlignedSize = (sizeof(ConstBufferPerObject) + 255) & ~255;

ID3D12Resource* constBufferUploadHeaps[frameBufferCnt];

UINT8* cbvGPUAdress[frameBufferCnt];

DirectX::XMFLOAT4X4 camProjMat;
DirectX::XMFLOAT4X4 camViewMat;

DirectX::XMFLOAT4 camPos;
DirectX::XMFLOAT4 camTarget;
DirectX::XMFLOAT4 camUp;

DirectX::XMFLOAT4X4 cube1WorldMat;
DirectX::XMFLOAT4X4 cube1RotMat;
DirectX::XMFLOAT4 cube1Pos;

DirectX::XMFLOAT4X4 cube2WorldMat;
DirectX::XMFLOAT4X4 cube2RotMat;
DirectX::XMFLOAT4 cube2PosOffset;

int numCubeIndices;
*/

bool appIsRunning = true;

// Geometry definitions
struct Vertex {
	Vertex() : pos(0, 0, 0), color(1, 1, 1, 1) {}
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

// Colors
const float rtvClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

// Geometry
std::array<Vertex, 4> triangleVertices;
std::array<DWORD, 6> triangleIndices;

/*
struct ConstBufferPerObject {
	DirectX::XMFLOAT4X4 wvpMat;
};
*/