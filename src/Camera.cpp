#include "Camera.h"


Camera::Camera()
{
}


Camera::~Camera()
{
}

void Camera::SetProjMat(const XMMATRIX& projMat)
{
	projMatrix = projMat;
	XMStoreFloat4x4(&this->projMat, projMatrix);
}

void Camera::SetPos(const XMFLOAT4& pos)
{
	this->pos = pos;
}

void Camera::SetTarget(const XMFLOAT4& target)
{
	this->target = target;
}

void Camera::SetUp(const XMFLOAT4& up)
{
	this->up = up;
}

void Camera::SetViewMat(const XMMATRIX& viewMat)
{
	XMStoreFloat4x4(&this->viewMat, viewMat);
}

void Camera::BuildViewMat()
{
	XMVECTOR vPos = XMLoadFloat4(&pos);
	XMVECTOR vTarget = XMLoadFloat4(&target);
	XMVECTOR vUp = XMLoadFloat4(&up);

	viewMatrix = XMMatrixLookAtRH(vPos, vTarget, vUp);

	XMStoreFloat4x4(&viewMat, viewMatrix);
}

void Camera::Move(const XMFLOAT4 dir)
{
	XMVECTOR posVec = XMLoadFloat4(&pos);
	XMVECTOR dirVec = XMLoadFloat4(&dir);

	posVec = XMVectorAdd(posVec, dirVec);
	XMStoreFloat4(&pos, posVec);
}

XMMATRIX Camera::GetTransposedWvpMat(const XMMATRIX& worldMat)
{
	return XMMatrixTranspose(worldMat * viewMatrix * projMatrix);
}

const DirectX::XMMATRIX Camera::GetTransposedProjMat()
{
	return XMMatrixTranspose(projMatrix);
}

const DirectX::XMMATRIX Camera::GetTransposedViewMat()
{
	return XMMatrixTranspose(viewMatrix);
}

void Camera::Update()
{

}
