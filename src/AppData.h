#pragma once

#include "Common.h"
#include "Camera.h"
#include "D3dDeviceResources.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct PointLight
{
	XMFLOAT4 pos;
	XMFLOAT3 att;
	float range;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	float specularPower;
	XMFLOAT3 padding; // 16 byte boundary
};

struct Material {
	XMFLOAT4 emissive;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	float specularPower;
	float specularIntensity;
	XMFLOAT2 padding; // 16 byte boundary
};

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

struct ConstBufferPs {
	PointLight pointLight;
	XMFLOAT4 eyePos;
	float padding[36];
};

struct ConstBufferPsMaterial {
	Material mat;
	float padding[44];
};

const UINT ConstBufferPsAlignedSize = (sizeof(ConstBufferPs) + 255) & ~255;

PointLight pointLight;

D3dDeviceResources* deviceResources;

ComPtr<ID3D12GraphicsCommandList> comList;

ComPtr<ID3D12DescriptorHeap> mainDescriptorHeap;

ComPtr<ID3D12Resource> planeVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW planeVertexBufferView;
ComPtr<ID3D12Resource> planeIndexBuffer;
D3D12_INDEX_BUFFER_VIEW planeIndexBufferView;

// Cube
ComPtr<ID3D12Resource> cubeVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW cubeVertexBufferView;
ComPtr<ID3D12Resource> cubeIndexBuffer;
D3D12_INDEX_BUFFER_VIEW cubeIndexBufferView;

// Sphere
ComPtr<ID3D12Resource> sphereVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW sphereVertexBufferView;
ComPtr<ID3D12Resource> sphereIndexBuffer;
D3D12_INDEX_BUFFER_VIEW sphereIndexBufferView;

ComPtr<ID3D12DescriptorHeap> mainDescHeap[frameBufferCnt];
ComPtr<ID3D12Resource> constBufUplHeap[frameBufferCnt];
ConstBuffer cbColorMultData;
UINT8* cbColorMultGpuAddr[frameBufferCnt];

UINT8* cbPerObjGpuAddr[frameBufferCnt];
ConstBufferPerObj cbPerObject;
ComPtr<ID3D12Resource> constBufPerObjUplHeap[frameBufferCnt];

UINT8* cbPsAddr[frameBufferCnt];
ConstBufferPs cbPs;
ComPtr<ID3D12Resource> cbPsUplHeap[frameBufferCnt];

UINT8* cbPsMatAddr[frameBufferCnt];
ConstBufferPsMaterial cbPsMat;
ComPtr<ID3D12Resource> cbPsMatUplHeap[frameBufferCnt];

Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateObject;

ComPtr<ID3DBlob> vertexShader;
ComPtr<ID3DBlob> pixelShader;

const std::wstring shaderPath = L"src/shaders/";
const LPCWSTR vertexShaderStr = L"vertexshader.hlsl";
const LPCWSTR pixelShaderStr = L"pixelshader.hlsl";

Camera* camera;

Material mat;

XMFLOAT4 light1Offset;

// Texture
ComPtr<ID3D12Resource> checkerboardTexture;
const UINT checkerboardTexWidth = 32;
const UINT checkerboardTexHeight = 32;
const UINT checkerboardTexSizeInBytes = 4;

ComPtr<ID3D12Resource> texBufUploadHeap;

UINT numCubeIndices;

bool appIsRunning = true;

const UINT sphereSliceCount = 20;
const UINT sphereStackCount = 20;

// Geometry definitions
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct VertexTex {
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};

struct VertexTexNorm {
	XMFLOAT3 pos;
	XMFLOAT2 tex;
	XMFLOAT3 norm;
};

struct Mesh {
	std::vector<VertexTexNorm> vertices;
	std::vector<UINT32> indices;
};

struct Entity {
	XMFLOAT4X4 worldMat;
	XMFLOAT4X4 rotMat;
	XMFLOAT4 pos;
};

Entity cube1;
Entity cube2;
Entity plane1;
Entity plane2;
Entity plane3;
Entity sphere;
XMMATRIX pLight1RotMat;

Mesh cubeMesh;
Mesh planeMesh;
Mesh sphereMesh;

D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

D3D12_INPUT_ELEMENT_DESC inputElementDescTex[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

D3D12_INPUT_ELEMENT_DESC inputElementDescTexNorm[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

// Geometry
//std::array<DWORD, 6> triangleIndices;
VertexTex cubeVerticesTex[24];
VertexTexNorm cubeVerticesTexNorm[24];
//Vertex triangleVertices[4];
VertexTexNorm triangleVerticesTexNorm[4];
std::array<DWORD, 36> cubeIndices;