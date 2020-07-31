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

	GameObject m_WoodCrate;
	GameObject m_Ground;
	GameObject m_House;
	GameObject m_Floor;
	GameObject m_Sphere;
	std::vector<GameObject> m_Walls;

	Material m_ShadowMat;
	Material m_Material;

	BasicEffect m_BasicEffect;
	
	ComPtr<VLMData> m_pVLMData ;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;

	ObjReader m_ObjReader;
};

