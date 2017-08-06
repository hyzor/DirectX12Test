#include "Camera.h"


Camera::Camera()
{
	m_right = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_pos = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_target = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_up = { 0.0f, 1.0f, 0.0f, 1.0f };
	m_viewIsDirty = true;
}


Camera::~Camera()
{
}

void Camera::SetProjMat(const XMMATRIX& projMat)
{
	m_projMatrix = projMat;
	XMStoreFloat4x4(&this->m_proj, m_projMatrix);
}

void Camera::SetPos(const XMFLOAT4& pos)
{
	m_pos = pos;
}

void Camera::SetTarget(const XMFLOAT4& target)
{
	m_target = target;
}

void Camera::SetUp(const XMFLOAT4& up)
{
	m_up = up;
}

void Camera::SetViewMat(const XMMATRIX& viewMat)
{
	XMStoreFloat4x4(&m_view, viewMat);
}

void Camera::SetLens(float fovY, float aspectRatio, float zNear, float zFar)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspectRatio, zNear, zFar);
	m_projMatrix = proj;

	XMStoreFloat4x4(&m_proj, m_projMatrix);
}

void Camera::BuildViewMat()
{
	XMVECTOR pos = XMLoadFloat4(&m_pos);
	XMVECTOR target = XMLoadFloat4(&m_target);
	XMVECTOR up = XMLoadFloat4(&m_up);

	m_viewMatrix = XMMatrixLookAtLH(pos, target, up);

	XMStoreFloat4x4(&m_view, m_viewMatrix);
}

void Camera::UpdateViewMat()
{
	if (!m_viewIsDirty)
		return;

	XMVECTOR rightVec = XMLoadFloat4(&m_right);
	XMVECTOR upVec = XMLoadFloat4(&m_up);
	XMVECTOR targetVec = XMLoadFloat4(&m_target);
	XMVECTOR posVec = XMLoadFloat4(&m_pos);

	// Keep Up and Right orthogonal to each other and Target
	targetVec = XMVector4Normalize(targetVec);
	upVec = XMVector4Normalize(XMVector3Cross(targetVec, rightVec));
	rightVec = XMVector3Cross(upVec, targetVec);

	// Calculate values for last column
	float x = -XMVectorGetX(XMVector3Dot(posVec, rightVec));
	float y = -XMVectorGetX(XMVector3Dot(posVec, upVec));
	float z = -XMVectorGetX(XMVector3Dot(posVec, targetVec));

	// Store updated Right, Up and Target vectors
	XMStoreFloat4(&m_right, rightVec);
	XMStoreFloat4(&m_up, upVec);
	XMStoreFloat4(&m_target, targetVec);

	m_view(0, 0) = m_right.x;
	m_view(1, 0) = m_right.y;
	m_view(2, 0) = m_right.z;
	m_view(3, 0) = x;

	m_view(0, 1) = m_up.x;
	m_view(1, 1) = m_up.y;
	m_view(2, 1) = m_up.z;
	m_view(3, 1) = y;

	m_view(0, 2) = m_target.x;
	m_view(1, 2) = m_target.y;
	m_view(2, 2) = m_target.z;
	m_view(3, 2) = z;

	m_view(0, 3) = 0.0f;
	m_view(1, 3) = 0.0f;
	m_view(2, 3) = 0.0f;
	m_view(3, 3) = 1.0f;

	m_viewMatrix = XMLoadFloat4x4(&m_view);

	m_viewIsDirty = false;
}

void Camera::Move(const XMFLOAT4 dir)
{
	XMVECTOR posVec = XMLoadFloat4(&m_pos);
	XMVECTOR dirVec = XMLoadFloat4(&dir);

	posVec = XMVectorAdd(posVec, dirVec);
	XMStoreFloat4(&m_pos, posVec);

	m_viewIsDirty = true;
}

XMMATRIX Camera::GetTransposedWvpMat(const XMMATRIX& worldMat)
{
	return XMMatrixTranspose(worldMat * m_viewMatrix * m_projMatrix);
}

const DirectX::XMMATRIX Camera::GetTransposedProjMat()
{
	return XMMatrixTranspose(m_projMatrix);
}

const DirectX::XMMATRIX Camera::GetTransposedViewMat()
{
	return XMMatrixTranspose(m_viewMatrix);
}

void Camera::Update()
{

}

void Camera::Strafe(float dist)
{
	XMVECTOR distVec = XMVectorReplicate(dist);
	XMVECTOR rightVec = XMLoadFloat4(&m_right);
	XMVECTOR posVec = XMLoadFloat4(&m_pos);
	XMStoreFloat4(&m_pos, XMVectorMultiplyAdd(distVec, rightVec, posVec));

	m_viewIsDirty = true;
}

void Camera::Walk(float dist)
{
	XMVECTOR distVec = XMVectorReplicate(dist);
	XMVECTOR targetVec = XMLoadFloat4(&m_target);
	XMVECTOR posVec = XMLoadFloat4(&m_pos);
	XMStoreFloat4(&m_pos, XMVectorMultiplyAdd(distVec, targetVec, posVec));

	m_viewIsDirty = true;
}
