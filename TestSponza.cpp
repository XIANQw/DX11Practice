#include "TestSponza.h"

using namespace DirectX;

TestSponza::TestSponza(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_CameraMode(CameraMode::Free),
	m_ShadowMat(),
	m_Material(),
	m_ObjReader(),
	m_IsWireframeMode(false) {
}


TestSponza::~TestSponza() {

}

bool TestSponza::Init() {
	if (!D3DApp::Init()) return false;

	RenderStates::InitAll(m_pd3dDevice.Get());

	if(!m_BasicEffect.SetVSShader3D(m_pd3dDevice.Get(), L"HLSL\\Ex13_VS3D.hlsl")) return false;
	if (!m_BasicEffect.SetPSShader3D(m_pd3dDevice.Get(), L"HLSL\\Ex13_PS3D.hlsl")) return false;;

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!InitResource()) return false;

	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::MODE_RELATIVE);

	return true;
}



void TestSponza::OnResize() {
	D3DApp::OnResize();

	if (m_pCamera != nullptr) {
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}


void TestSponza::UpdateScene(float dt)
{
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	auto cam1st = std::dynamic_pointer_cast<FPSCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<TPSCamera>(m_pCamera);

	if (m_CameraMode == CameraMode::Free) {
		// FPS mode
		if (keyState.IsKeyDown(Keyboard::W)) {
			cam1st->MoveForward(dt * 15.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			cam1st->MoveForward(-dt * 15.0f);
		}
		if (keyState.IsKeyDown(Keyboard::D)) cam1st->Strafe(dt * 15.0f);
		if (keyState.IsKeyDown(Keyboard::A)) cam1st->Strafe(-dt * 15.0f);

		// 限制摄像机在固定范围内 x(-8.9, 8.9), z(-8.9, 8.9), y(0, 8.9) y不能为负
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-80.0f, 0.0f, -80.0f, 0.0f), XMVectorReplicate(80.0f)));
		cam1st->SetPosition(adjustPos);

		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
		
	}


	// 更新观察矩阵
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	// 重置滚轮
	m_pMouse->ResetScrollWheelValue();

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::R)) {
		m_IsWireframeMode = !m_IsWireframeMode;
	}

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape)) {
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}

}


void TestSponza::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::White));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	/************************
	1. 绘制物体
	*************************/

	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
	m_BasicEffect.SetTextureUsed(true);
	m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetShadowState(false);
	m_Sponza.SetMaterial(m_Material);

	/************************
	2. 绘制阴影
	*************************/
	if (!m_IsWireframeMode) {
		m_BasicEffect.SetShadowState(true);	// 阴影开启
		m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);
		m_Sponza.SetMaterial(m_ShadowMat);
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}

	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_Sponza.SetMaterial(m_Material);

	HR(m_pSwapChain->Present(0, 0));
}


bool TestSponza::InitResource()
{

	/*******************
		初始化纹理
	********************/
	// 初始材质
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	// 阴影材质，形成阴影效果
	m_Material = material;
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	// 读取sponza
	m_ObjReader.Read(L"Model\\SponzaUV.mbo", L"Model\\SponzaUV.obj");
	m_Sponza.SetModel(Model(m_pd3dDevice.Get(), m_ObjReader));

	//// 获取房屋包围盒
	XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
	BoundingBox houseBox = m_Sponza.GetLocalBoundingBox();
	houseBox.Transform(houseBox, S);

	Transform& houseTransform = m_Sponza.GetTransform();
	houseTransform.SetScale(0.015f, 0.015f, 0.015f);
	houseTransform.SetPosition(0.0f, -(houseBox.Center.y - houseBox.Extents.y + 1.0f), 0.0f);


	/*******************************
		初始化摄像机, 默认第三人称视角
	********************************/
	m_CameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FPSCamera>(new FPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->SetPosition(0.0f, 6.0f, -10.0f);

	/**********************************************
		设置观察矩阵和观察位置，设置平截头体和投影矩阵
	***********************************************/

	//m_BasicEffect.SetWorldMatrix(XMMatrixIdentity());
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	//m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));

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
	//// 灯光，range会阴影光圈的范围
	PointLight pointLight(XMFLOAT3(0.0f, 10.0f, 0.0f));
	PointLight pointLight1(XMFLOAT3(15.0f, 10.0f, 0.0f));
	PointLight pointLight2(XMFLOAT3(-15.0f, 10.0f, 0.0f));
	pointLight.range = pointLight1.range = pointLight2.range = 10.0f;
	pointLight.position = XMFLOAT3(30.0f, 60.0f, 0.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 100.0f;
	m_BasicEffect.SetPointLight(0, pointLight);
	m_BasicEffect.SetPointLight(1, pointLight1);
	m_BasicEffect.SetPointLight(2, pointLight2);


	/*
		设置阴影矩阵
		稍微高一点位置以显示阴影
	*/
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
		XMVectorSet(pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f)));
	//m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));



	return true;
}
