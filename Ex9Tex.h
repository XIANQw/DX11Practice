#ifndef Ex9Tex_H
#define Ex9Tex_H

#include "d3dApp.h"
#include "light.h"
#include "Geometry.h"
#include <vector>


class Ex9Tex : public D3DApp {

    enum class ShowMode {
        WoodCrate, FireAnim
    };

public:

    struct VSConstantBuffer {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct PSConstantBuffer {
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        Material material;
        int numDirLight;
        int numPointLight;
        int numSpotLight;
        float pad;
        DirectX::XMFLOAT4 eyePos;
    };

public:
    Ex9Tex(HINSTANCE hInstance);
    virtual ~Ex9Tex();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();


protected:
    bool InitEffect();
    bool InitResource();

    template<class VertexType>
    bool ResetMesh(const Geometry::MeshData<VertexType>& meshData);

    ComPtr<ID3D11InputLayout> m_pVertexLayout2D;	// 2D顶点输入布局
    ComPtr<ID3D11InputLayout> m_pVertexLayout3D;	// 3D顶点输入布局
    
    ComPtr<ID3D11Buffer> m_pVertexBuffer;		// 顶点缓冲区
    ComPtr<ID3D11Buffer> m_pIndexBuffer;        // 索引缓冲区
    ComPtr<ID3D11Buffer> m_pConstantBuffers[2];     // 常量缓冲区
    UINT m_IndexCount;
    UINT m_CurrFrame;
    ShowMode m_CurrMode;

    ComPtr<ID3D11VertexShader> m_pVertexShader2D;	// 2D顶点着色器
    ComPtr<ID3D11VertexShader> m_pVertexShader3D;	// 3D顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader2D;	// 2D像素着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader3D;	// 3D像素着色器

    VSConstantBuffer m_VSConstantBuffer; // 储存了各种变换矩阵
    PSConstantBuffer m_PSConstantBuffer;

    ComPtr<ID3D11SamplerState> m_pSamplerState;

    // 光照模型
    PointLight m_PointLight;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe; //光栅化状态，切换线框模式

    // 纹理资源
    ComPtr<ID3D11ShaderResourceView> m_pWoodCrate;
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_pFireAnims;

};

#endif