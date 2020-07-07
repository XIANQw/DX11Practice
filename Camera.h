#pragma once
#include "d3d11.h"
#include "Transform.h"

/**************************************************
	在Transform的基础上实现了两种摄像机类 FPS 和 TPS
***************************************************/

class Camera
{
public:
	Camera() = default;
	virtual ~Camera() = 0;

	/*
		获取摄像机位置
	*/
	DirectX::XMVECTOR GetPositionXM() const;
	DirectX::XMFLOAT3 GetPosition() const;

	// 获取X轴旋转欧拉角弧度
	float GetRotationX() const;
	//获取Y轴旋转欧拉角弧度
	float GetRotationY() const;

	//获取摄像机坐标轴向量
	DirectX::XMVECTOR GetRightAxisXM() const;
	DirectX::XMFLOAT3 GetRightAxis() const;
	DirectX::XMVECTOR GetUpAxisXM() const;
	DirectX::XMFLOAT3 GetUpAxis() const;
	DirectX::XMVECTOR GetLookAxisXM() const;
	DirectX::XMFLOAT3 GetLookAxis() const;

	/*获取矩阵*/
	DirectX::XMMATRIX GetViewXM() const;
	DirectX::XMMATRIX GetProjXM() const;
	DirectX::XMMATRIX GetViewProjXM() const;

	// 获取视口
	D3D11_VIEWPORT GetViewPort() const;
	
	// 设置平截头体
	void SetFrustum(float fovY, float aspec, float nearZ, float farZ);

	// 设置视口
	void SetViewPort(const D3D11_VIEWPORT& viewport);
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

protected:
	Transform m_Transform = {};
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;

	// 当前视口
	D3D11_VIEWPORT m_ViewPort = {};
};

class FPSCamera :public Camera {
public:
	FPSCamera() = default;
	~FPSCamera() override;
	// 设置摄像机位置
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& pos);
	
	// 设置摄像机朝向
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookTo(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);

	// 平移
	void Strafe(float d);
	// 平面移动
	void Walk(float d);
	// 前进
	void MoveForward(float d);
	// 上下观察，+值向上，-值向下
	void Pitch(float rad);
	// 左右观察 +值向左，-值向右
	void RotateY(float rad);

};

class TPSCamera :public Camera {
public:
	TPSCamera() = default;
	~TPSCamera() override;

	// 获取摄像机跟踪物体的位置
	DirectX::XMFLOAT3 GetTargetPosition() const;
	// 获取与跟踪物体的距离
	float GetDistance() const;
	// 绕物体的x轴旋转(俯仰角，限制在60°)
	void RotateX(float rad);
	// 绕物体的y轴旋转(偏移角）
	void RotateY(float rad);
	// 接近物体
	void Approach(float dist);
	// 设置俯仰角(限制在60°)
	void SetRotationX(float rad);
	// 设置偏移角
	void SetRotationY(float rad);
	// 设置并绑定待跟踪物体的位置
	void SetTarget(const DirectX::XMFLOAT3& target);
	// 设置初始距离
	void SetDistance(float dist);
	// 设置最小和最远距离
	void SetDistMinMax(float minDist, float maxFloat);

private:
	// 目标的位置
	DirectX::XMFLOAT3 m_Target = {};
	// 摄像机和目标的距离
	float m_Distance = 0.0f;
	// 摄像机离目标的最大和最小距离
	float m_MinDist = 0.0f, m_MaxDist = 0.0f;
};