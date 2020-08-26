#pragma once


#include <string.h>
#include <DirectXMath.h>

struct Material {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 reflect;
	Material() = default;
};

// 平行光模型
struct DirectionalLight
{
    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;
    DirectX::XMFLOAT3 direction;
	float foo; //显示的使结构体16字节对齐
};


struct PointLight {
	PointLight() {
		memset(this, 0, sizeof(PointLight));
	}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	// 打包成4D向量 (pos, range)
	DirectX::XMFLOAT3 position;
	float range;

	// 4D 向量 (A0, A1, A2, pad)
	DirectX::XMFLOAT3 att;
	float pad;
};


struct SpotLight
{
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	// 打包成4D向量: (position, range)
	DirectX::XMFLOAT3 position;
	float range;

	// 打包成4D向量: (direction, spot)
	DirectX::XMFLOAT3 direction;
	float spot;

	// 打包成4D向量: (att, pad)
	DirectX::XMFLOAT3 att;
	float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

