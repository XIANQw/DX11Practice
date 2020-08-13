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
	GameObject m_Box;
	std::vector<GameObject> m_Spheres;

	Material m_ShadowMat;
	Material m_Material;

	BasicEffect m_BasicEffect;
	std::vector<PointLight> m_PointLightArray;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;
	ObjReader m_ObjReader;
	Importer m_Importer;
	
	bool m_IsWireframeMode;
	bool m_UseSH;
	bool m_UseTexture;
	bool m_UseLight;

	bool m_UseDirLight;
	bool m_UsePointLight;
	int m_UseBrickId = 0;
	int m_SHMode = 0;

	wchar_t m_Text[512];
	DirectX::XMVECTORF32 m_BackGroundColor;
	INT32 m_Speed;
	INT32 m_SphereSpeed;
	std::vector<INT8> m_SpheresDirection;
};

