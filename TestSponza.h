#pragma once
#include "d3dApp.h"
#include "Geometry.h"
#include "Transform.h"
#include "CBuffer.h"
#include "Camera.h"
#include "GameObject.h"

/*************************
	Ex13 实现了阴影
**************************/



class TestSponza :
	public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FPS, TPS, Free, Observe };


public:
	TestSponza(HINSTANCE hInstance);
	virtual ~TestSponza();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


protected:
	bool InitResource();

	GameObject m_Sponza;
	std::vector<GameObject> m_Walls;

	bool m_IsWireframeMode;

	Material m_ShadowMat;
	Material m_Material;

	BasicEffect m_BasicEffect;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;

	ObjReader m_ObjReader;
};

