#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
#include "d3dx12.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

// D3D12 stuff
const int frameBufferCnt = 3; // Three for triple buffering
ID3D12Device* d3dDevice;
IDXGISwapChain3* d3dSwapChain;
ID3D12CommandQueue* d3dComQueue;
ID3D12DescriptorHeap* d3dRtvDescriptorHeap;
ID3D12Resource* d3dRenderTargets[frameBufferCnt];
ID3D12CommandAllocator* d3dComAlloc[frameBufferCnt];
ID3D12GraphicsCommandList* d3dComList;
ID3D12Fence* d3dFence[frameBufferCnt];
ID3D12RootSignature* d3dRootSignature;
ID3D12PipelineState* d3dPipelineStateObject;
HANDLE d3dFenceEvent;
UINT64 d3dFenceValue[frameBufferCnt];
int d3dFrameIdx;
int d3dRtvDesciptorSize;

ID3D12Resource* vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ID3D12Resource* indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

D3D12_VIEWPORT d3dViewport;
D3D12_RECT d3dScissorRect;

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

struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, z) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

/*
struct ConstBufferPerObject {
	DirectX::XMFLOAT4X4 wvpMat;
};
*/