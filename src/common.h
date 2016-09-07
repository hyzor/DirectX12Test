#pragma once

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
HANDLE d3dFenceEvent;
UINT64 d3dFenceValue[frameBufferCnt];
int d3dFrameIdx;
int d3dRtvDesciptorSize;

bool appIsRunning = true;