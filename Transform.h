#pragma once
#include <DirectXMath.h>
/*****************************************
*	Transform.h							 *
* Transform类用于实现摄像机的转向和移动		 *
******************************************/

class Transform
{
public:
	Transform() = default;
	Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position);
	~Transform() = default;

	// 获取缩放矩阵
	DirectX::XMFLOAT3 GetScale() const;
	DirectX::XMVECTOR GetScaleXM() const;

	// 获取旋转欧拉角
	DirectX::XMFLOAT3 GetRotation() const;
	DirectX::XMVECTOR GetRotationXM() const;

	// 获取对象位置
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMVECTOR GetPositionXM() const;

	// 获取右方向轴
	DirectX::XMFLOAT3 GetRightAxis() const;
	DirectX::XMVECTOR GetRightAxisXM() const;

	// 获取上方向轴
	DirectX::XMFLOAT3 GetUpAxis() const;
	DirectX::XMVECTOR GetUpAxisXM() const;

	// 获取前方向轴
	DirectX::XMFLOAT3 GetForwardAxis() const;
	DirectX::XMVECTOR GetForwardAxisXM() const;


	// 获取世界变换矩阵
	DirectX::XMFLOAT4X4 GetLocalToWorldMatrix() const;
	DirectX::XMMATRIX GetLocalToWorldMatrixXM() const;

	// 获取世界变换逆矩阵
	DirectX::XMFLOAT4X4 GetWorldToLocalMatrix() const;
	DirectX::XMMATRIX GetWorldToLocalMatrixXM() const;

	// 设置缩放矩阵
	void SetScale(const DirectX::XMFLOAT3& scale);
	void SetScale(float x, float y, float z);

	// 设置对象欧拉角(弧度制), 对象以z-x-y轴顺序旋转，为了避免 "万向节死锁"
	void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	void SetRotation(float x, float y, float z);

	// 设置对象位置
	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetPosition(float x, float y, float z);

	// 按欧拉角旋转对象
	void Rotate(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	// 绕坐标轴旋转
	void RotateAxis(const DirectX::XMFLOAT3& axis, float radian);
	// 绕点旋转
	void RotateAround(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian);

	// 沿着某一方向前进
	void Translate(const DirectX::XMFLOAT3 & direction, float magnitude);

	// 观察某一点
	void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = {0.0f, 1.0f, 0.0f});
	// 沿着某方向看
	void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f,0.0f });

	DirectX::XMFLOAT3 GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix);

private:
	DirectX::XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_Rotation = {};
	DirectX::XMFLOAT3 m_Position = {};
};

