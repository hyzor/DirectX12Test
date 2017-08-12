#pragma once

#include <map>
#include <forward_list>

#include "Shared.h"
#include "Camera.h"
#include "D3dDeviceResources.h"
#include "Mesh.h"
#include "Entity.h"
#include "Texture.h"
#include "Light.h"

using namespace Microsoft::WRL;
using namespace DirectX;

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
	ConstBufferPs()
	{

	}

	PointLight pointLight;
	XMFLOAT4 eyePos;
	float padding[36];
};

struct ConstBufferPsMaterial {
	Material mat;
	float padding[44];
};

const UINT ConstBufferPsAlignedSize = (sizeof(ConstBufferPs) + 255) & ~255;

D3dDeviceResources* deviceResources;

ComPtr<ID3D12GraphicsCommandList> comList;

ComPtr<ID3D12DescriptorHeap> mainDescriptorHeap;

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

bool appIsRunning = true;

XMMATRIX pLight1RotMat;

std::map<std::string, std::shared_ptr<Mesh>> meshes;
std::map<std::string, std::shared_ptr<Texture>> textures;
std::forward_list<Entity> entities;
std::forward_list<Light> lights;