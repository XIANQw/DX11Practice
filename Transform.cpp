#include "Transform.h"

using namespace DirectX;

XMFLOAT3 Transform::GetScale() const {
	return m_Scale;
}

XMVECTOR Transform::GetScaleXM() const {
	return XMLoadFloat3(&m_Scale);
}

XMFLOAT3 Transform::GetRotation() const {
	return m_Rotation;
}


XMVECTOR Transform::GetRotationXM() const {
	return XMLoadFloat3(&m_Rotation);
}


XMFLOAT3 Transform::GetPosition() const {
	return m_Position;
}

XMVECTOR Transform::GetPositionXM() const {
	return XMLoadFloat3(&m_Position);
}

XMFLOAT3 Transform::GetRightAxis() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 right;
	XMStoreFloat3(&right, tmp.r[0]);
	return right;
}

XMVECTOR Transform::GetRightAxisXM() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	return tmp.r[0];
}

XMFLOAT3 Transform::GetUpAxis() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 up;
	XMStoreFloat3(&up, tmp.r[1]);
	return up;
}

XMVECTOR Transform::GetUpAxisXM() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	return tmp.r[1];
}

XMFLOAT3 Transform::GetForwardAxis() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, tmp.r[2]);
	return forward;
}

XMVECTOR Transform::GetForwardAxisXM() const {
	XMMATRIX tmp = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	return tmp.r[2];
}

XMFLOAT4X4 Transform::GetLocalToWorldMatrix() const {
	XMFLOAT4X4 res;
	XMStoreFloat4x4(&res, GetLocalToWorldMatrixXM());
	return res;
}

XMMATRIX Transform::GetLocalToWorldMatrixXM() const {
	XMVECTOR scaleVec = XMLoadFloat3(&m_Scale);
	XMVECTOR rotateVec = XMLoadFloat3(&m_Rotation);
	XMVECTOR posVec = XMLoadFloat3(&m_Position);
	XMMATRIX res = XMMatrixScalingFromVector(scaleVec) * XMMatrixRotationRollPitchYawFromVector(rotateVec) * XMMatrixTranslationFromVector(posVec);
	return res;
}

XMFLOAT4X4 Transform::GetWorldToLocalMatrix() const {
	XMFLOAT4X4 res;
	XMStoreFloat4x4(&res, GetWorldToLocalMatrixXM());
	return res;
}

XMMATRIX Transform::GetWorldToLocalMatrixXM() const {
	XMMATRIX w2l = XMMatrixInverse(nullptr, GetLocalToWorldMatrixXM());
	return w2l;
}

void Transform::SetScale(const XMFLOAT3& scale) {
	m_Scale = scale;
}

void Transform::SetScale(float x, float y, float z) {
	m_Scale = XMFLOAT3(x, y, z);
}

void Transform::SetRotation(const XMFLOAT3& eulerAnglesInRadian) {
	m_Rotation = eulerAnglesInRadian;
}

void Transform::SetRotation(float x, float y, float z) {
	m_Rotation = XMFLOAT3(x, y, z);
}

void Transform::SetPosition(const XMFLOAT3& position) {
	m_Position = position;
}

void Transform::SetPosition(float x, float y, float z) {
	m_Position = XMFLOAT3(x, y, z);
}

/*
	transition = magnitude * direction; new Position = current position + transition; 
*/
void Transform::Translate(const XMFLOAT3& direction, float magnitude) {
	XMVECTOR curPosVec = XMLoadFloat3(&m_Position);
	XMVECTOR directVec = XMVector3Normalize(XMLoadFloat3(&direction));
	XMVECTOR newPosVec = XMVectorMultiplyAdd(XMVectorReplicate(magnitude), directVec, curPosVec);
	XMStoreFloat3(&m_Position, newPosVec);
}

void Transform::LookAt(const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMMATRIX View = XMMatrixLookAtLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMMATRIX InvView = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, InvView);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::LookTo(const XMFLOAT3& direction, const XMFLOAT3& up)
{
	XMMATRIX View = XMMatrixLookToLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&direction), XMLoadFloat3(&up));
	XMMATRIX InvView = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, InvView);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::Rotate(const XMFLOAT3& eulerAnglesInRadian) {
	XMVECTOR curRotation = XMLoadFloat3(&m_Rotation);
	XMVECTOR newRotation = XMLoadFloat3(&eulerAnglesInRadian);
	XMStoreFloat3(&m_Rotation, (XMVectorAdd(curRotation, newRotation)));
}

/*
	绕轴旋转欧拉角radian
	当前的欧拉角变换矩阵 XMMatrixRotationRollPitchYawFromVector(curRotation)
	绕轴axis旋转radian的变换矩阵 XMMatrixRotationAxis(XMLoadFloat3(&axis), radian)
*/
void Transform::RotateAxis(const XMFLOAT3& axis, float radian) {
	XMVECTOR curRotation = XMLoadFloat3(&m_Rotation);
	XMMATRIX newRotation = XMMatrixRotationRollPitchYawFromVector(curRotation) * XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
	XMFLOAT4X4 rotMatix;
	XMStoreFloat4x4(&rotMatix, newRotation);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatix);
}


void Transform::RotateAround(const XMFLOAT3& point, const XMFLOAT3& axis, float radian)
{
	XMVECTOR rotationVec = XMLoadFloat3(&m_Rotation);
	XMVECTOR positionVec = XMLoadFloat3(&m_Position);
	XMVECTOR centerVec = XMLoadFloat3(&point);

	// 以point作为原点进行旋转
	XMMATRIX RT = XMMatrixRotationRollPitchYawFromVector(rotationVec) * XMMatrixTranslationFromVector(positionVec - centerVec);
	RT *= XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
	RT *= XMMatrixTranslationFromVector(centerVec);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, RT);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
	XMStoreFloat3(&m_Position, RT.r[3]);
}

XMFLOAT3 Transform::GetEulerAnglesFromRotationMatrix(const XMFLOAT4X4& rotationMatrix)
{
	// 通过旋转矩阵反求欧拉角
	float c = sqrtf(1.0f - rotationMatrix(2, 1) * rotationMatrix(2, 1));
	// 防止r[2][1]出现大于1的情况
	if (isnan(c))
		c = 0.0f;
	XMFLOAT3 rotation;
	rotation.z = atan2f(rotationMatrix(0, 1), rotationMatrix(1, 1));
	rotation.x = atan2f(-rotationMatrix(2, 1), c);
	rotation.y = atan2f(rotationMatrix(2, 0), rotationMatrix(2, 2));
	return rotation;
}

