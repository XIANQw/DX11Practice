#pragma once


#include <string.h>
#include <DirectXMath.h>

struct Material {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 reflect;
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

