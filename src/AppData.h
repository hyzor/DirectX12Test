#pragma once

#include <map>
#include <forward_list>

#include "Shared.h"
#include "Camera.h"
#include "D3dDeviceResources.h"
#include "Mesh.h"
#include "Entity.h"
#include "Texture.h"
#include "components/Light.h"
#include "Timer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

// Win32 properties
HWND hwnd = NULL;
LPCTSTR wndName = L"D3d12Test";
LPCTSTR wndTitle = L"D3d12Test";

int wndWidth = 1024;
int wndHeight = 768;
bool wndFullScreen = false;

bool appIsRunning = true;
bool appIsPaused = false;
bool wndIsMinimized = false;
bool wndIsMaximized = false;
bool wndIsResizing = false;

// Constant buffers
struct ConstBuffer {
	XMFLOAT4 colorMult;
};
const UINT ConstBufferAlignedSize = (sizeof(ConstBuffer) + 255) & ~255;

struct ConstBufferPerObj {
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
};
const UINT ConstBufferPerObjAlignedSize = (sizeof(ConstBufferPerObj) + 255) & ~255;

struct ConstBufferPs {
	ConstBufferPs()
	{

	}

	PointLightStruct pointLight[32];
	XMFLOAT4 eyePos;
	int numPointLights;
};

struct ConstBufferPsMaterial {
	Material mat;
};

const UINT ConstBufferPsAlignedSize = (sizeof(ConstBufferPs) + 255) & ~255;

D3dDeviceResources* deviceResources;

Timer* timer;

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

// Texture
ComPtr<ID3D12Resource> checkerboardTexture;
const UINT checkerboardTexWidth = 32;
const UINT checkerboardTexHeight = 32;
const UINT checkerboardTexSizeInBytes = 4;

ComPtr<ID3D12Resource> texBufUploadHeap;

XMMATRIX pLight1RotMat;

std::map<std::string, std::shared_ptr<Mesh>> meshes;
std::map<std::string, std::shared_ptr<Texture>> textures;
std::forward_list<Entity> entities;
std::forward_list<Light> lights;

XMFLOAT2 lastMousePos;