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
	void VisualizeVLM();


protected:
	bool InitResource();
	bool InitVLM();

	GameObject m_Sponza;
	GameObject m_Box;
	GameObject m_plane;
	std::vector<Transform> m_DynamicTransform;
	GameObject m_Sample;
	std::vector<std::vector<Transform>> m_TransformData;

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
	bool m_isVisulizeVLM;
	int m_SHMode;

	bool m_ShowHelp;
	bool m_isControlObj;

	wchar_t m_Text[1024];
	DirectX::XMVECTORF32 m_BackGroundColor;
	INT32 m_Speed;
	INT32 m_SphereSpeed;
	INT32 m_TargetDistance;
	std::vector<INT8> m_SpheresDirection;

	std::vector<const wchar_t*> m_SHRepositoies;
	INT32 m_SHFileIndex;
	ComPtr<ID3D11Texture3D> m_pTex3D;

};

