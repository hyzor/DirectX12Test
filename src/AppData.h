#pragma once

#include <forward_list>
#include <unordered_map>

#include "Shared.h"
#include "Camera.h"
#include "D3dDeviceResources.h"
#include "Mesh.h"
#include "Entity.h"
#include "Texture.h"
#include "components/Light.h"
#include "components/PointLight.h"
#include "components/Component.h"
#include "Timer.h"
#include "ConstantBuffer.h"

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
struct ConstBufferPerObj 
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
};
const UINT ConstBufferPerObjAlignedSize = (sizeof(ConstBufferPerObj) + 255) & ~255;

struct ConstBufferPs 
{
	ConstBufferPs() {}

	PointLightStruct pointLight[16];
	XMFLOAT4 eyePos;
	UINT numPointLights;
};

struct ConstBufferPsMaterial 
{
	Material mat;
};

const UINT ConstBufferPsAlignedSize = (sizeof(ConstBufferPs) + 255) & ~255;

D3dDeviceResources* deviceResources;

Timer* timer;

ComPtr<ID3D12GraphicsCommandList> comList;

ComPtr<ID3D12DescriptorHeap> mainDescriptorHeap;

std::unordered_map<std::string, std::shared_ptr<IConstantBuffer>> constantBuffersMap;

std::vector<ComPtr<ID3D12DescriptorHeap>> mainDescHeap;
// ConstBuffer cbColorMultData;
// std::array<UINT8*, frameBufferCnt> cbColorMultGpuAddr;

// std::array<UINT8*, frameBufferCnt> cbPerObjGpuAddr;
// ConstBufferPerObj cbPerObject;
// std::array<ComPtr<ID3D12Resource>, frameBufferCnt> constBufPerObjUplHeap;

std::array<UINT8*, frameBufferCnt> cbPsAddr;
ConstBufferPs cbPs;
std::array<ComPtr<ID3D12Resource>, frameBufferCnt> cbPsUplHeap;

std::array<UINT8*, frameBufferCnt> cbPsMatAddr;
ConstBufferPsMaterial cbPsMat;
std::array<ComPtr<ID3D12Resource>, frameBufferCnt> cbPsMatUplHeap;

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

std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
std::forward_list<Entity> entities;

Entity* pLightEntity;

XMFLOAT2 lastMousePos;

// Random
std::random_device randDevice;
std::mt19937 mt19937(randDevice());
std::uniform_real_distribution<float> dist01(0.0f, 1.0f);