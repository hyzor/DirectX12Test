#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	void SetProjMat(const XMMATRIX& projMat);
	void SetPos(const XMFLOAT4& pos);
	void SetTarget(const XMFLOAT4& target);
	void SetUp(const XMFLOAT4& up);
	void SetViewMat(const XMMATRIX& viewMat);
	void BuildViewMat();

	XMMATRIX GetTransposedWvpMat(const XMMATRIX& worldMat);

	void Update();

private:
	XMFLOAT4X4 projMat;
	XMFLOAT4X4 viewMat;

	XMMATRIX projMatrix;
	XMMATRIX viewMatrix;

	XMFLOAT4 pos;
	XMFLOAT4 target;
	XMFLOAT4 up;
};

