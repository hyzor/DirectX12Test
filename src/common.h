#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <array>
#include <wrl.h>

#include "Camera.h"

using namespace Microsoft::WRL;
using namespace DirectX;

// Macros
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

#define PI 3.14159265358979323846f

// Constant buffers
struct ConstBuffer {
	XMFLOAT4 colorMult;
	XMFLOAT4 padding[15]; // Padded to fit 256 bytes (! Wasted CPU cycles !)
};
const UINT ConstBufferAlignedSize = (sizeof(ConstBuffer) + 255) & ~255;

struct ConstBufferPerObj {
	XMFLOAT4X4 wvpMat;
	XMFLOAT4 padding[12]; // Padded to fit 256 bytes (! Wasted CPU cycles !)
};
const UINT ConstBufferPerObjAlignedSize = (sizeof(ConstBufferPerObj) + 255) & ~255;

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

// Cube
ComPtr<ID3D12Resource> cubeVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW cubeVertexBufferView;
ComPtr<ID3D12Resource> cubeIndexBuffer;
D3D12_INDEX_BUFFER_VIEW cubeIndexBufferView;

D3D12_VIEWPORT d3dViewport;
D3D12_RECT d3dScissorRect;

ComPtr<ID3D12Resource> depthStencilBuffer;
ComPtr<ID3D12DescriptorHeap> dsDescHeap;

ComPtr<ID3D12DescriptorHeap> mainDescHeap[frameBufferCnt];
ComPtr<ID3D12Resource> constBufUplHeap[frameBufferCnt];
ConstBuffer cbColorMultData;
UINT8* cbColorMultGpuAddr[frameBufferCnt];

UINT8* cbPerObjGpuAddr[frameBufferCnt];
ConstBufferPerObj cbPerObject;
ComPtr<ID3D12Resource> constBufPerObjUplHeap[frameBufferCnt];

ComPtr<ID3DBlob> vertexShader;
ComPtr<ID3DBlob> pixelShader;

const std::wstring shaderPath = L"src/shaders/";
const LPCWSTR vertexShaderStr = L"vertexshader.hlsl";
const LPCWSTR pixelShaderStr = L"pixelshader.hlsl";

Camera* camera;

/*
ID3D12Resource* constBufferUploadHeaps[frameBufferCnt];

UINT8* cbvGPUAdress[frameBufferCnt];
*/

UINT numCubeIndices;

bool appIsRunning = true;

// Geometry definitions
struct Vertex {
	Vertex() : pos(0, 0, 0), color(1, 1, 1, 1) {}
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct Cube {
	DirectX::XMFLOAT4X4 worldMat;
	DirectX::XMFLOAT4X4 rotMat;
	DirectX::XMFLOAT4 pos;
};

Cube cube1;
Cube cube2;

D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

// Colors
const float rtvClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

// Geometry
std::array<Vertex, 8> triangleVertices;
std::array<DWORD, 6> triangleIndices;
std::array<Vertex, 24> cubeVertices;
std::array<DWORD, 36> cubeIndices;

/*
struct ConstBufferPerObject {
	DirectX::XMFLOAT4X4 wvpMat;
};
*/