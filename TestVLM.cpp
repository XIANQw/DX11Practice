#include "TestVLM.h"

using namespace DirectX;

TestVLM::TestVLM(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_CameraMode(CameraMode::FPS),
	m_ShadowMat(),
	m_Material(),
	m_ObjReader(),
	m_Speed(500),
	m_BackGroundColor(Colors::White){
}


TestVLM::~TestVLM() {

}


bool TestVLM::Init() {

	if (!D3DApp::Init()) return false;
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.SetVSShader3D(m_pd3dDevice.Get(), L"HLSL\\TestVLM_VS3D.hlsl")) return false;
	if (!m_BasicEffect.SetPSShader3D(m_pd3dDevice.Get(), L"HLSL\\TestVLM_PS3D.hlsl")) return false;

	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get())) return false;

	if (!InitVLM()) return false;
	if (!InitResource()) return false;
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::Mode::MODE_RELATIVE);

	return true;
}


void TestVLM::OnResize() {
	D3DApp::OnResize();

	if (m_pCamera != nullptr) {
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 10000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}


void TestVLM::UpdateScene(float dt) {
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	auto cam1st = std::dynamic_pointer_cast<FPSCamera>(m_pCamera);

	if (m_CameraMode == CameraMode::Free) {
		// FPS mode
		if (keyState.IsKeyDown(Keyboard::W)) {
			cam1st->MoveForward(dt * m_Speed);
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			cam1st->MoveForward(-dt * m_Speed);
		}
		if (keyState.IsKeyDown(Keyboard::D)) cam1st->Strafe(dt * m_Speed);
		if (keyState.IsKeyDown(Keyboard::A)) cam1st->Strafe(-dt * m_Speed);

		if (m_KeyboardTracker.IsKeyPressed(Keyboard::T)) {
			m_Speed += 500;
		}

		if (m_KeyboardTracker.IsKeyPressed(Keyboard::Y)) {
			m_Speed -= 500;
		}
		m_Speed < 500 ? m_Speed = 500 : m_Speed;

		// 限制摄像机在固定范围内 x(-8.9, 8.9), z(-8.9, 8.9), y(0, 8.9) y不能为负
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMVECTOR VolumeMinVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeMin);
		XMVECTOR VolumeSizeVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeSize);
		XMVECTOR VolumeMaxVec = XMVectorAdd(VolumeMinVec, VolumeSizeVec);
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(),VolumeMinVec, VolumeMaxVec));
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
		m_BackGroundColor = m_IsWireframeMode ? Colors::Black : Colors::White;
	}

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape)) {
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}
}

void TestVLM::DrawScene() {
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&m_BackGroundColor));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	/************************
	1. 绘制物体
	*************************/
	if (m_IsWireframeMode) {
		m_BasicEffect.SetWireFrameWode(m_pd3dImmediateContext.Get());
		m_BasicEffect.SetTextureUsed(false);

		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		/*
		3. 绘制世界包围盒
		*/
		m_Box.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	} else{
		m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
		m_BasicEffect.SetTextureUsed(true);
		m_Sponza.SetMaterial(m_Material);
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	HR(m_pSwapChain->Present(0, 0));

}

void CreateTexture3D(ID3D11Device* device, ID3D11DeviceContext* context, INT32 depth, INT32 width, INT32 height, DXGI_FORMAT format, const std::vector<UINT8>& srcData, ID3D11ShaderResourceView** outSRV) {
	ID3D11Texture3D* pTex3D;
	
	D3D11_TEXTURE3D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Depth = depth;
	texDesc.MipLevels = 1;
	texDesc.Format = format;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.CPUAccessFlags = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subResData;
	subResData.pSysMem = srcData.data();
	subResData.SysMemPitch = width * 4;
	subResData.SysMemSlicePitch = width * height * 4;
	HR(device->CreateTexture3D(&texDesc, &subResData, &pTex3D));

	D3D11_SHADER_RESOURCE_VIEW_DESC tex3DViewDesc;
	ZeroMemory(&tex3DViewDesc, sizeof(tex3DViewDesc));
	tex3DViewDesc.Format = format;
	tex3DViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	tex3DViewDesc.Texture3D.MipLevels = 1;
	tex3DViewDesc.Texture3D.MostDetailedMip = 0;
	HR(device->CreateShaderResourceView(pTex3D, &tex3DViewDesc, outSRV));
}

bool TestVLM::InitVLM() {
	m_Importer.ImportFile(L"D:\\brickData_1596541525");
	if (!m_Importer.Read())
		return false;

	m_Importer.TransformData();

	const VLMData& vlmData = m_Importer.vlmData;
	m_BasicEffect.SetVLMBrickSize(m_Importer.VLMSetting.BrickSize);
	m_BasicEffect.SetVLMIndirectionTextureSize(XMFLOAT3(vlmData.textureDimension.x, vlmData.textureDimension.y, vlmData.textureDimension.z));
	m_BasicEffect.SetVLMBrickTexelSize(XMFLOAT3(vlmData.brickDataDimension.x, vlmData.brickDataDimension.y, vlmData.brickDataDimension.z));
	XMVECTOR VolumeSizeVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeSize);
	XMVECTOR InvVolumeSizeVec = XMVectorReciprocal(VolumeSizeVec);
	XMFLOAT3 InvVolumeSize;
	XMStoreFloat3(&InvVolumeSize, InvVolumeSizeVec);
	m_BasicEffect.SetVLMWorldToUVScale(InvVolumeSize);
	XMVECTOR VolumeMinVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeMin);
	XMFLOAT3 VLMWorldToUVAdd;
	XMStoreFloat3(&VLMWorldToUVAdd, XMVectorMultiply(VolumeMinVec, InvVolumeSizeVec));
	m_BasicEffect.SetVLMWorldToUVAdd(VLMWorldToUVAdd);



	ComPtr<ID3D11ShaderResourceView> SRV;
	CreateTexture3D(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(), 16, 16, 16, vlmData.indirectionTexture.Format, vlmData.indirectionTexture.data, SRV.GetAddressOf());
	m_BasicEffect.SetTexture3D(SRV.Get());

	CreateTexture3D(m_pd3dDevice.Get(),
		m_pd3dImmediateContext.Get(),
		vlmData.brickDataDimension.z,
		vlmData.brickDataDimension.x,
		vlmData.brickDataDimension.y,
		vlmData.brickData.AmbientVector.Format,
		vlmData.brickData.AmbientVector.data,
		SRV.GetAddressOf());
	m_BasicEffect.SetTexture3D(SRV.Get());

	for (int i = 0; i < 6; i++) {
		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[i].Format,
			vlmData.brickData.SHCoefficients[i].data,
			SRV.GetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());
	}

	return true;
}


bool TestVLM::InitResource() {
	const XMFLOAT3& VolumeSize = m_Importer.VLMSetting.VolumeSize;
	const XMFLOAT3& VolumeMin = m_Importer.VLMSetting.VolumeMin;
	XMVECTOR VolumeMinVec = XMLoadFloat3(&VolumeMin);
	XMVECTOR VolumeSizeVec = XMLoadFloat3(&VolumeSize);
	XMVECTOR BoundCentreVec = XMVectorAdd(VolumeMinVec, XMVectorDivide(VolumeSizeVec, XMVectorSet(2.0f,2.0f,2.0f,2.0f)));
	m_Box.SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateBox(VolumeSize.x, VolumeSize.y, VolumeSize.z)));
	XMFLOAT3 BoundCentre; XMStoreFloat3(&BoundCentre, BoundCentreVec);
	m_Box.GetTransform().SetPosition(BoundCentre);

	/*******************
		初始化纹理
	********************/
	// 初始材质
	Material material{};
	material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);
	m_Material = material;


	// 读取sponza
	m_ObjReader.Read(L"Model\\SponzaUV.mbo", L"Model\\SponzaUV.obj");
	m_Sponza.SetModel(Model(m_pd3dDevice.Get(), m_ObjReader));

	//// 获取房屋包围盒
	XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
	BoundingBox houseBox = m_Sponza.GetLocalBoundingBox();
	houseBox.Transform(houseBox, S);

	Transform& houseTransform = m_Sponza.GetTransform();
	houseTransform.SetScale(1.0f, 1.0f, 1.0f);
	houseTransform.SetPosition(0.0f, -(houseBox.Center.y - houseBox.Extents.y + 1.0f), 0.0f);


	/*******************************
		初始化摄像机, 默认第三人称视角
	********************************/
	m_CameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FPSCamera>(new FPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 10000.0f);
	camera->SetPosition(0.0f, 6.0f, 0.0f);

	/**********************************************
		设置观察矩阵和观察位置，设置平截头体和投影矩阵
	***********************************************/

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
	PointLight pointLight(XMFLOAT3(0.0f, 4000.0f, 0.0f));
	PointLight pointLight1(XMFLOAT3(15.0f, 4000.0f, 0.0f));
	PointLight pointLight2(XMFLOAT3(-15.0f, 4000.0f, 0.0f));
	pointLight.position = XMFLOAT3(30.0f, 60.0f, 0.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = pointLight1.range = pointLight2.range = 7000.0f;
	m_BasicEffect.SetPointLight(0, pointLight);
	m_BasicEffect.SetPointLight(1, pointLight1);
	m_BasicEffect.SetPointLight(2, pointLight2);

	/*
		设置阴影矩阵
		稍微高一点位置以显示阴影
	*/
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
		XMVectorSet(pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f)));

	return true;
}

