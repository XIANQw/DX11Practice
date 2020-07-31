#include "TestVLM.h"

using namespace DirectX;

bool TestVLM::Init() {
	
	if (!D3DApp::Init()) return false;
	RenderStates::InitAll(m_pd3dDevice.Get());
	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get())) return false;

	if (!InitResource()) return false;
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);

	return true;
}
	
bool TestVLM::InitResource() {
	/*******************
		初始化纹理
	********************/
	ComPtr<ID3D11ShaderResourceView> texture;
	// 初始材质
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);
	m_Material = material;

	// 阴影材质，形成阴影效果
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	// 读取木箱贴图，设置木箱的材质和贴图以及所处的世界空间位置
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodCrate.SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateBox()));
	m_WoodCrate.GetTransform().SetPosition(0.0f, 0.01f, 0.0f);
	m_WoodCrate.SetTexture(texture.Get());
	m_WoodCrate.SetMaterial(material);


	/*******************************
		初始化摄像机, 默认第三人称视角
	********************************/
	m_CameraMode = CameraMode::FPS;
	auto camera = std::shared_ptr<FPSCamera>(new FPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->SetPosition(0.0f, 6.0f, -10.0f);

	/**********************************************
		设置观察矩阵和观察位置，设置平截头体和投影矩阵
	***********************************************/
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());
	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());

	/******************
	*  初始化默认光照	  *
	*******************/
	// 环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	// 灯光，range会阴影光圈的范围
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, 20.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 100.0f;
	m_BasicEffect.SetPointLight(0, pointLight);

	/*
		设置阴影矩阵
		稍微高一点位置以显示阴影
	*/
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
		XMVectorSet(pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f)));
	//m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

	
	return true;
}