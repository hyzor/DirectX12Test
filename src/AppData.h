#pragma once

#include <windows.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <array>
#include <wrl.h>

#include "Camera.h"
#include "D3dDeviceResources.h"

using namespace Microsoft::WRL;
using namespace DirectX;

// Macros
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define PI 3.14159265358979323846f

// Constant buffers
struct ConstBuffer {
	XMFLOAT4 colorMult;
	float padding[60];
};
const UINT ConstBufferAlignedSize = (sizeof(ConstBuffer) + 255) & ~255;

struct ConstBufferPerObj {
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
	float padding[16];
};
const UINT ConstBufferPerObjAlignedSize = (sizeof(ConstBufferPerObj) + 255) & ~255;

D3dDeviceResources* deviceResources;

ComPtr<ID3D12GraphicsCommandList> comList;

ComPtr<ID3D12DescriptorHeap> mainDescriptorHeap;

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

// Cube
ComPtr<ID3D12Resource> cubeVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW cubeVertexBufferView;
ComPtr<ID3D12Resource> cubeIndexBuffer;
D3D12_INDEX_BUFFER_VIEW cubeIndexBufferView;

ComPtr<ID3D12DescriptorHeap> mainDescHeap[frameBufferCnt];
ComPtr<ID3D12Resource> constBufUplHeap[frameBufferCnt];
ConstBuffer cbColorMultData;
UINT8* cbColorMultGpuAddr[frameBufferCnt];

UINT8* cbPerObjGpuAddr[frameBufferCnt];
ConstBufferPerObj cbPerObject;
ComPtr<ID3D12Resource> constBufPerObjUplHeap[frameBufferCnt];

Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateObject;

ComPtr<ID3DBlob> vertexShader;
ComPtr<ID3DBlob> pixelShader;

const std::wstring shaderPath = L"src/shaders/";
const LPCWSTR vertexShaderStr = L"vertexshader.hlsl";
const LPCWSTR pixelShaderStr = L"pixelshader.hlsl";

Camera* camera;

// Texture
ComPtr<ID3D12Resource> checkerboardTexture;
const UINT checkerboardTexWidth = 32;
const UINT checkerboardTexHeight = 32;
const UINT checkerboardTexSizeInBytes = 4;

ComPtr<ID3D12Resource> texBufUploadHeap;

UINT numCubeIndices;

bool appIsRunning = true;

// Geometry definitions
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

// Geometry definitions
struct VertexTex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
	XMFLOAT2 tex;
};

struct Cube {
	XMFLOAT4X4 worldMat;
	XMFLOAT4X4 rotMat;
	XMFLOAT4 pos;
};

Cube cube1;
Cube cube2;

D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

D3D12_INPUT_ELEMENT_DESC inputElementDescTex[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

// Geometry
std::array<DWORD, 6> triangleIndices;
VertexTex cubeVerticesTex[24];
Vertex triangleVertices[4];
std::array<DWORD, 36> cubeIndices;