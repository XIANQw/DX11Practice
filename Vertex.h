﻿#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

struct VertexPosNormalColor {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 noraml;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};

struct VertexPosNormal {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 noraml;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
};

struct VertexPosTex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
};


struct VertexPosNormalTex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
};


struct Instances {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX worldInvTranspose;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[10];
};


struct VertexPosNormalTangentTex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC inputLayout[4];
};

void CalculeTangent(std::vector<VertexPosNormalTangentTex>& triangle);

