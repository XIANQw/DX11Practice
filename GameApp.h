﻿#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "light.h"
#include "Geometry.h"

class GameApp : public D3DApp {
public:

    struct VSConstantBuffer {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct PSConstantBuffer {
        DirectionalLight dirLight;
        PointLight pointLight;
        SpotLight spotLight;
        Material material;
        DirectX::XMFLOAT4 eyePos;
    };

public:
    GameApp(HINSTANCE hInstance);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();


private:
    bool InitEffect();
    bool InitResource();
    bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);

    ComPtr<ID3D11InputLayout> m_pVertexLayout;	// 顶点输入布局
    ComPtr<ID3D11Buffer> m_pVertexBuffer;		// 顶点缓冲区
    ComPtr<ID3D11Buffer> m_pIndexBuffer;        // 索引缓冲区
    ComPtr<ID3D11Buffer> m_pConstantBuffer[2];     // 常量缓冲区
    UINT m_IndexCount;

    ComPtr<ID3D11VertexShader> m_pVertexShader;	// 顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader;	// 像素着色器
    VSConstantBuffer m_VSConstantBuffer; // 储存了各种变换矩阵
    PSConstantBuffer m_PSConstantBuffer;
    
    // 光照模型
    DirectionalLight m_DirLight;
    PointLight m_PointLight;
    SpotLight m_SpotLight;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe; //光栅化状态，切换线框模式
    bool m_IsWireframeMode;
    
};

#endif