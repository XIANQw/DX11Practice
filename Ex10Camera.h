#pragma once
#include "d3dApp.h"
#include "Geometry.h"
#include "Transform.h"
#include "CBuffer.h"
#include "Camera.h"
class Ex10Camera :
    public D3DApp
{
public:

    class GameObject
    {
    public:
        GameObject();

        // 获取物体变换
        Transform& GetTransform();
        // 获取物体变换
        const Transform& GetTransform() const;

        // 设置缓冲区
        template<class VertexType, class IndexType>
        void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

        // 设置纹理
        void SetTexture(ID3D11ShaderResourceView* texture);

        // 绘制
        void Draw(ID3D11DeviceContext* deviceContext);

    private:
        Transform m_Transform;                          // 世界矩阵
        ComPtr<ID3D11ShaderResourceView> m_pTexture;        // 纹理
        ComPtr<ID3D11Buffer> m_pVertexBuffer;               // 顶点缓冲区
        ComPtr<ID3D11Buffer> m_pIndexBuffer;                // 索引缓冲区
        UINT m_VertexStride;                                // 顶点字节大小
        UINT m_IndexCount;                                  // 索引数目   
    };

    // 摄像机模式
    enum class CameraMode { FPS, TPS, Free, Observe };


public:
    Ex10Camera(HINSTANCE hInstance);
    virtual ~Ex10Camera();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();


protected:
    bool InitEffect();
    bool InitResource();

    ComPtr<ID3D11InputLayout> m_pVertexLayout2D;	// 2D顶点输入布局
    ComPtr<ID3D11InputLayout> m_pVertexLayout3D;	// 3D顶点输入布局
    ComPtr<ID3D11Buffer> m_pConstantBuffers[4];     // 常量缓冲区

    GameObject m_WoodCrate;
    GameObject m_Floor;
    std::vector<GameObject> m_Walls;

    ComPtr<ID3D11VertexShader> m_pVertexShader2D;	// 2D顶点着色器
    ComPtr<ID3D11VertexShader> m_pVertexShader3D;	// 3D顶点着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader2D;	// 2D像素着色器
    ComPtr<ID3D11PixelShader> m_pPixelShader3D;	// 3D像素着色器

    CBChangesEveryFrame m_CBFrame;      // 帧缓冲区
    CBChangesOnResize m_CBOnResize;     // 改变大小缓冲区
    CBChangesRarely m_CBRarely;         // 几乎不变缓冲区

    ComPtr<ID3D11SamplerState> m_pSamplerState; // 采样器

    // 光照模型
    PointLight m_PointLight;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe; //光栅化状态，切换线框模式

    std::shared_ptr<Camera> m_pCamera;
    CameraMode m_CameraMode;


};

