#include "Camera.h"

using namespace DirectX;

Camera::~Camera() {

}

XMVECTOR Camera::GetPositionXM() const{
    return m_Transform.GetPositionXM();
}

XMFLOAT3 Camera::GetPosition() const {
    return m_Transform.GetPosition();
}

float Camera::GetRotationX() const {
    return m_Transform.GetRotation().x;
}

float Camera::GetRotationY() const {
    return m_Transform.GetRotation().y;
}

//获取摄像机坐标轴向量
XMVECTOR Camera::GetRightAxisXM() const
{
    return m_Transform.GetRightAxisXM();
}
XMFLOAT3 Camera::GetRightAxis() const {
    return m_Transform.GetRightAxis();
}
XMVECTOR Camera::GetUpAxisXM() const
{
    return m_Transform.GetUpAxisXM();
}
XMFLOAT3 Camera::GetUpAxis() const {
    return m_Transform.GetUpAxis();
}
XMVECTOR Camera::GetLookAxisXM() const
{
    return m_Transform.GetForwardAxisXM();
}
XMFLOAT3 Camera::GetLookAxis() const {
    return m_Transform.GetForwardAxis();
}

/*获取矩阵*/
XMMATRIX Camera::GetViewXM() const {
    return m_Transform.GetWorldToLocalMatrixXM();
}
XMMATRIX Camera::GetProjXM() const {
    return XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
}
XMMATRIX Camera::GetViewProjXM() const {
    return GetViewXM() * GetProjXM();
}

// 获取视口
D3D11_VIEWPORT Camera::GetViewPort() const {
    return m_ViewPort;
}

// 设置平截头体
void Camera::SetFrustum(float fovY, float aspec, float nearZ, float farZ) {
    m_FovY = fovY;
    m_Aspect = aspec;
    m_NearZ = nearZ;
    m_FarZ = farZ;
}

// 设置视口
void Camera::SetViewPort(const D3D11_VIEWPORT& viewport) {
    m_ViewPort = viewport;
}
void Camera::SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth) {
    D3D11_VIEWPORT viewPort;
    ZeroMemory(&viewPort, sizeof(viewPort));
    viewPort.TopLeftX = topLeftX;
    viewPort.TopLeftY = topLeftY;
    viewPort.Width = width;
    viewPort.Height = height;
    viewPort.MinDepth = minDepth;
    viewPort.MaxDepth = maxDepth;
    m_ViewPort = viewPort;
}



/***********************
    第一人称摄像机
************************/

FPSCamera::~FPSCamera() {

}

void FPSCamera::SetPosition(float x, float y, float z)
{
    SetPosition(XMFLOAT3(x, y, z));
}

void FPSCamera::SetPosition(const XMFLOAT3& pos)
{
    m_Transform.SetPosition(pos);
}

// 设置摄像机朝向
void FPSCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up) {
    m_Transform.SetPosition(pos);
    m_Transform.LookAt(target, up);
}

void FPSCamera::LookTo(const XMFLOAT3& pos, const XMFLOAT3& to, const XMFLOAT3& up) {
    m_Transform.SetPosition(pos);
    m_Transform.LookTo(to, up);
}

// 左右平移
void FPSCamera::Strafe(float d) {
    m_Transform.Translate(m_Transform.GetRightAxis(), d);
}
// 第一人称前后移动
void FPSCamera::Walk(float d) {
    XMVECTOR rightVec = m_Transform.GetRightAxisXM();
    XMVECTOR frontVec = XMVector3Normalize(XMVector3Cross(rightVec, g_XMIdentityR1));
    XMFLOAT3 front;
    XMStoreFloat3(&front, frontVec);
    m_Transform.Translate(front, d);
}

// 前进
void FPSCamera::MoveForward(float d) {
    m_Transform.Translate(m_Transform.GetForwardAxis(), d);
}
// 上下观察，+值向上，-值向下
void FPSCamera::Pitch(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    // 俯仰角 绕x轴
    rotation.x += rad;
    // 为了防止过度抬头， 将俯仰角限制在了+-70°
    if (rotation.x > XM_PI * 7 / 18)
        rotation.x = XM_PI * 7 / 18;
    else if (rotation.x < -XM_PI * 7 / 18)
        rotation.x = -XM_PI * 7 / 18;
    m_Transform.SetRotation(rotation);
}

// 左右观察 +值向左，-值向右
void FPSCamera::RotateY(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    rotation.y = XMScalarModAngle(rotation.y + rad);
    m_Transform.SetRotation(rotation);
}

/***********************
    第三人称摄像机
************************/


TPSCamera::~TPSCamera()
{
}


XMFLOAT3 TPSCamera::GetTargetPosition() const {
    return m_Target;
}


float TPSCamera::GetDistance() const {
    return m_Distance;
}

void TPSCamera::RotateX(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    rotation.x += rad;
    if (rotation.x > XM_PI / 3)
        rotation.x = XM_PI / 3;
    else if (rotation.x < 0.0f)
        rotation.x = 0.0f;
    m_Transform.SetRotation(rotation);
    m_Transform.SetPosition(m_Target);
    m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
}

void TPSCamera::RotateY(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    rotation.y = XMScalarModAngle(rotation.y + rad);

    m_Transform.SetRotation(rotation);
    m_Transform.SetPosition(m_Target);
    m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
}

void TPSCamera::Approach(float dist) {
    m_Distance += dist;
    if (m_Distance < m_MinDist) m_Distance = m_MinDist;
    else if (m_Distance > m_MaxDist) m_Distance = m_MaxDist;

    m_Transform.SetPosition(m_Target);
    m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
}


void TPSCamera::SetRotationX(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    rotation.x = rad;
    if (rotation.x < -XM_PI / 3)
        rotation.x = -XM_PI / 3;
    else if (rotation.x > 0.0f)
        rotation.x = 0.0f;

    m_Transform.SetRotation(rotation);
    m_Transform.SetPosition(m_Target);
    m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
}

void TPSCamera::SetRotationY(float rad) {
    XMFLOAT3 rotation = m_Transform.GetRotation();
    rotation.y = XMScalarModAngle(rad);
    m_Transform.SetRotation(rotation);
    m_Transform.SetPosition(m_Target);
    m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
}

void TPSCamera::SetTarget(const XMFLOAT3& pos) {
    m_Target = pos;
}

void TPSCamera::SetDistance(const float dist) {
    m_Distance = dist;
}

void TPSCamera::SetDistMinMax(float minDist, float maxDist) {
    m_MinDist = minDist;
    m_MaxDist = maxDist;
}
