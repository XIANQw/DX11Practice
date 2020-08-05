#pragma once
#include "d3dApp.h"
#include "Transform.h"
#include "CBuffer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Importer.h"

class TestVLM :public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FPS, TPS, Free, Observe };


public:
	TestVLM(HINSTANCE hInstance);
	virtual ~TestVLM();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


protected:
	bool InitResource();
	bool InitVLM();

	GameObject m_Sponza;
	GameObject m_WoodCrate;
	GameObject m_Ground;
	GameObject m_Box;
	GameObject m_Floor;
	GameObject m_Sphere;
	std::vector<GameObject> m_Walls;

	Material m_ShadowMat;
	Material m_Material;

	BasicEffect m_BasicEffect;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;
	bool m_IsWireframeMode;
	ObjReader m_ObjReader;
	Importer m_Importer;
	
	DirectX::XMVECTORF32 m_BackGroundColor;
	INT32 m_Speed;
};

