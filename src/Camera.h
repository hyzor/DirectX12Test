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
	void SetLens(float fovY, float aspectRatio, float zNear, float zFar);
	void BuildViewMat();
	void UpdateViewMat();

	void Move(const XMFLOAT4 dir);

	XMMATRIX GetTransposedWvpMat(const XMMATRIX& worldMat);
	const XMMATRIX GetTransposedProjMat();
	const XMMATRIX GetTransposedViewMat();
	const XMFLOAT4 GetPos() { return m_pos; }

	void Update();

	void Strafe(float dist);
	void Walk(float dist);

	void Pitch(float angle);
	void Yaw(float angle);

private:
	XMFLOAT4X4 m_proj;
	XMFLOAT4X4 m_view;

	XMMATRIX m_projMatrix;
	XMMATRIX m_viewMatrix;

	XMFLOAT4 m_pos;
	XMFLOAT4 m_target;
	XMFLOAT4 m_up;
	XMFLOAT4 m_right;

	bool m_viewIsDirty;
};

