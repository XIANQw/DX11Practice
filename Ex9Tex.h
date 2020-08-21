#ifndef Ex9Tex_H
#define Ex9Tex_H

#include "d3dApp.h"
#include "light.h"
#include "CBuffer.h"
#include <vector>

/*****************************************
    Ex9:
       实现了贴图的载入，以及2D动画的实现。
******************************************/


class Ex9Tex : public D3DApp {

    enum class ShowMode {
        WoodCrate, FireAnim
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
    ComPtr<ID3D11Buffer> m_pConstantBuffers[4];     // 常量缓冲区
    UINT m_IndexCount;
    UINT m_CurrFrame;
    ShowMode m_CurrMode;

    ComPtr<ID3D11VertexShader> m_pVertexShader2D;	// 2D顶点着色器
    ComPtr<ID3D11VertexShader> m_pVertexShader3D;	// 3D顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader2D;	// 2D像素着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader3D;	// 3D像素着色器

    CBChangesEveryDrawing m_CBDrawing;
    CBChangesEveryFrame m_CBFrame;
    CBChangesOnResize m_CBOnResize;
    CBChangesRarely m_CBRarely;

    ComPtr<ID3D11SamplerState> m_pSamplerState;

    // 光照模型
    PointLight m_PointLight;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe; //光栅化状态，切换线框模式

    // 纹理资源
    ComPtr<ID3D11ShaderResourceView> m_pWoodCrate;
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_pFireAnims;

};

#endif