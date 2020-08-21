#pragma once
#include "d3dApp.h"
#include "Transform.h"
#include "CBuffer.h"
#include "Camera.h"
#include "GameObject.h"

/*************************
    Ex13 实现了阴影
**************************/



class Ex13Shadow :
    public D3DApp
{
public:
    // 摄像机模式
    enum class CameraMode { FPS, TPS, Free, Observe };


public:
    Ex13Shadow(HINSTANCE hInstance);
    virtual ~Ex13Shadow();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();


protected:
    bool InitResource();

    GameObject m_WoodCrate;
    GameObject m_Floor;
    std::vector<GameObject> m_Walls;

    Material m_ShadowMat;
    Material m_WoodCrateMat;

    BasicEffect m_BasicEffect;

    std::shared_ptr<Camera> m_pCamera;
    CameraMode m_CameraMode;


};

