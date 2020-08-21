#include "TestVLM.h"

using namespace DirectX;

TestVLM::TestVLM(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_ShadowMat(),
	m_Material(),
	m_CameraMode(CameraMode::FPS),
	m_ObjReader(),
	m_Speed(500),
	m_BackGroundColor(Colors::White),
	m_IsWireframeMode(false),
	m_UseSH(true),
	m_UseTexture(true),
	m_UseLight(false),
	m_UseDirLight(false),
	m_UsePointLight(false),
	m_SHMode(true),
	m_isVisulizeVLM(false), 
	m_SphereSpeed(200),
	m_Text(L""),
	m_SHRepositoies{L"200", L"150", L"100", L"50" },
	m_SHFileIndex(0){
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


	m_BasicEffect.SetDebugName();
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::Mode::MODE_RELATIVE);

	return true;
}


void TestVLM::OnResize() {
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);
	// 释放D2D的相关资源
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();

	D3DApp::OnResize();

	// 为D2D创建DXGI表面渲染目标
	// 为D2D创建DXGI表面渲染目标
	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HR(m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.GetAddressOf()));
	surface.Reset();

	HR(m_pd2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		m_pColorBrush.GetAddressOf()));
	HR(m_pdwriteFactory->CreateTextFormat(L"宋体", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
		m_pTextFormat.GetAddressOf()));


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
	XMFLOAT3 curPos;
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


		// 限制摄像机在固定范围内 x(-8.9, 8.9), z(-8.9, 8.9), y(0, 8.9) y不能为负
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMVECTOR VolumeMinVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeMin);
		XMVECTOR VolumeSizeVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeSize);
		XMVECTOR VolumeMaxVec = XMVectorAdd(VolumeMinVec, VolumeSizeVec);
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(), VolumeMinVec, VolumeMaxVec));

		cam1st->SetPosition(adjustPos);
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
		BoundingBox houseBox = m_Sponza.GetBoundingBox();

		for (int i = 0; i < m_Spheres.size(); i++) {
			auto& trans = m_Spheres[i].GetTransform();
			curPos = trans.GetPosition();
			if (curPos.x > 1160.0f) m_SpheresDirection[i] = -1;
			else if (curPos.x < -1300.0f) m_SpheresDirection[i] = 1;
			curPos.x += m_SpheresDirection[i] * dt * m_SphereSpeed;
			trans.SetPosition(curPos);
		}
	}
	// 更新观察矩阵
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	// 重置滚轮
	m_pMouse->ResetScrollWheelValue();
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::U)) {
		m_Speed += 500;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Y)) {
		m_Speed -= 500;
	}
	m_Speed = max(200, m_Speed);

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::R)) {
		m_IsWireframeMode = !m_IsWireframeMode;
		m_BackGroundColor = m_IsWireframeMode ? Colors::Black : Colors::White;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::F)) {
		m_UseSH = !m_UseSH;
		m_BasicEffect.SetSHUsed(m_UseSH);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::T)) {
		m_UseTexture = !m_UseTexture;
		m_BasicEffect.SetTextureUsed(m_UseTexture);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::V)) {
		m_isVisulizeVLM = !m_isVisulizeVLM;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::N)) {
		m_SHFileIndex += 1;
		m_SHFileIndex = min(m_SHFileIndex, m_SHRepositoies.size() - 1); 
		m_BasicEffect.ClearTexture3D();
		InitVLM();
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::M)) {
		m_SHFileIndex -= 1;
		m_SHFileIndex = max(0, m_SHFileIndex);
		m_BasicEffect.ClearTexture3D();
		InitVLM();
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1)) {
		m_UseDirLight = !m_UseDirLight;
		m_BasicEffect.SetDirLightUsed(m_UseDirLight);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2)) {
		m_UsePointLight = !m_UsePointLight;
		m_BasicEffect.SetPointLightUsed(m_UsePointLight);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3)) {
		m_SHMode = 0;
		m_BasicEffect.SetSHMode(m_SHMode);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D4)) {
		m_SHMode = 1;
		m_BasicEffect.SetSHMode(m_SHMode);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D5)) {
		m_SHMode = 2;
		m_BasicEffect.SetSHMode(m_SHMode);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Add)) {
		m_SphereSpeed += 200;
		m_SphereSpeed = min(600, m_SphereSpeed);
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Subtract)) {
		m_SphereSpeed -= 200;
		m_SphereSpeed = max(0, m_SphereSpeed);
	}

	_snwprintf_s(m_Text, ARRAYSIZE(m_Text), ARRAYSIZE(m_Text) - 1, L"SHMode=%d, dirLight=%d, pointLight=%d, posW(%f,%f,%f), IsVisulizeVLM=%d, CameraSpeed=%d, SphereSpeed=%d, detailCellSize=%s",
		m_SHMode, m_UseDirLight, m_UsePointLight, cam1st->GetPosition().x, cam1st->GetPosition().y, cam1st->GetPosition().z, m_isVisulizeVLM, m_Speed, m_SphereSpeed, m_SHRepositoies[m_SHFileIndex]);

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
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		/*
		3. 绘制世界包围盒
		*/
		m_Box.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	else {
		m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
		m_Sponza.SetMaterial(m_Material);
		m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	m_BasicEffect.SetTextureUsed(false);
	for (auto& sphere : m_Spheres) {
		sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
	}
	if (m_isVisulizeVLM) {
		for (auto& sample : m_Samples) {
			sample.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		}
	}
	m_BasicEffect.SetTextureUsed(m_UseTexture);

	WriteInformation(std::wstring(m_Text));

	HR(m_pSwapChain->Present(0, 0));

}

XMFLOAT3 IndTexPosToWorldPos(const XMINT3& indTexPos, const FVolumetricLightmapSettings& vlmSetting, const XMINT3& indTexDimension) {
	XMVECTOR volumeMinVec = XMLoadFloat3(&vlmSetting.VolumeMin);
	XMVECTOR volumeSizeVec = XMLoadFloat3(&vlmSetting.VolumeSize);
	XMVECTOR indTexDimensionVec = XMLoadSInt3(&indTexDimension);
	XMVECTOR indTexPosVec = XMLoadSInt3(&indTexPos);
	// (indPos/indDimension * VolumeSize + VolumeMin)
	XMVECTOR worldPosVec = XMVectorAdd(XMVectorMultiply(XMVectorDivide(indTexPosVec, indTexDimensionVec), volumeSizeVec), volumeMinVec);
	XMFLOAT3 worldPos;
	XMStoreFloat3(&worldPos, worldPosVec);
	return worldPos;
}

void TestVLM::VisualizeVLM() {
	m_Samples.resize(m_Importer.m_BricksNum);

	INT32 SampleIndex = 0;
	for (INT32 depth = 0; depth < m_Importer.m_BricksByDepth.size(); depth++) {
		for (INT32 index = 0; index < m_Importer.m_BricksByDepth[depth].size(); index++) {
			const auto& brick = m_Importer.m_BricksByDepth[depth][index];
			float radius = 10.0f;
			radius = radius + (2 - brick.TreeDepth) * 40;
			m_Samples[SampleIndex].SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateSphere(radius)));
			m_Samples[SampleIndex++].GetTransform().SetPosition(IndTexPosToWorldPos(brick.IndirectionTexturePosition, m_Importer.VLMSetting, m_Importer.vlmData.textureDimension));
		}
	}

}




void CreateTexture3D(ID3D11Device* device, ID3D11DeviceContext* context, INT32 depth, INT32 width, INT32 height, DXGI_FORMAT format, const std::vector<UINT8>& srcData, ID3D11Texture3D** pTex3D, ID3D11ShaderResourceView** outSRV) {

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
	HR(device->CreateTexture3D(&texDesc, &subResData, pTex3D));

	D3D11_SHADER_RESOURCE_VIEW_DESC tex3DViewDesc;
	ZeroMemory(&tex3DViewDesc, sizeof(tex3DViewDesc));
	tex3DViewDesc.Format = format;
	tex3DViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	tex3DViewDesc.Texture3D.MipLevels = 1;
	tex3DViewDesc.Texture3D.MostDetailedMip = 0;
	HR(device->CreateShaderResourceView(*pTex3D, &tex3DViewDesc, outSRV));
}

bool TestVLM::InitVLM() {

	wchar_t VLMSettingRepo[128], brickByDepthRepo[128], indTexRepo[128], AmbientVecRepo[128], SH0Repo[128],
		SH1Repo[128], SH2Repo[128], SH3Repo[128], SH4Repo[128], SH5Repo[128];
	_snwprintf_s(brickByDepthRepo, ARRAYSIZE(brickByDepthRepo), ARRAYSIZE(brickByDepthRepo) - 1, L"Texture\\SHCoefs\\%s\\brickDataByDepth", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(VLMSettingRepo, ARRAYSIZE(VLMSettingRepo), ARRAYSIZE(VLMSettingRepo) - 1, L"Texture\\SHCoefs\\%s\\vlmSetting", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(indTexRepo, ARRAYSIZE(indTexRepo), ARRAYSIZE(indTexRepo) - 1, L"Texture\\SHCoefs\\%s\\indirectionTexture", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(AmbientVecRepo, ARRAYSIZE(AmbientVecRepo), ARRAYSIZE(AmbientVecRepo) - 1, L"Texture\\SHCoefs\\%s\\AmbientVector", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH0Repo, ARRAYSIZE(SH0Repo), ARRAYSIZE(SH0Repo) - 1, L"Texture\\SHCoefs\\%s\\SH0", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH1Repo, ARRAYSIZE(SH1Repo), ARRAYSIZE(SH1Repo) - 1, L"Texture\\SHCoefs\\%s\\SH1", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH2Repo, ARRAYSIZE(SH2Repo), ARRAYSIZE(SH2Repo) - 1, L"Texture\\SHCoefs\\%s\\SH2", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH3Repo, ARRAYSIZE(SH3Repo), ARRAYSIZE(SH3Repo) - 1, L"Texture\\SHCoefs\\%s\\SH3", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH4Repo, ARRAYSIZE(SH4Repo), ARRAYSIZE(SH4Repo) - 1, L"Texture\\SHCoefs\\%s\\SH4", m_SHRepositoies[m_SHFileIndex]);
	_snwprintf_s(SH5Repo, ARRAYSIZE(SH5Repo), ARRAYSIZE(SH5Repo) - 1, L"Texture\\SHCoefs\\%s\\SH5", m_SHRepositoies[m_SHFileIndex]);

	m_Importer.ImportFile(brickByDepthRepo, VLMSettingRepo,
		indTexRepo, AmbientVecRepo, SH0Repo, SH1Repo,SH2Repo,SH3Repo,SH4Repo,SH5Repo);

	if (!m_Importer.Read())
		return false;

	m_Importer.TransformData();

	const VLMData& vlmData = m_Importer.vlmData;
	m_BasicEffect.SetVLMBrickSize(m_Importer.VLMSetting.BrickSize);
	m_BasicEffect.SetVLMIndirectionTextureSize(XMFLOAT3(vlmData.textureDimension.x, vlmData.textureDimension.y, vlmData.textureDimension.z));
	m_BasicEffect.SetVLMBrickTexelSize(XMFLOAT3(1.0f / vlmData.brickDataDimension.x, 1.0f / vlmData.brickDataDimension.y, 1.0f / vlmData.brickDataDimension.z));
	XMVECTOR VolumeSizeVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeSize);
	XMVECTOR InvVolumeSizeVec = XMVectorReciprocal(VolumeSizeVec);
	XMFLOAT3 InvVolumeSize;
	XMStoreFloat3(&InvVolumeSize, InvVolumeSizeVec);
	m_BasicEffect.SetVLMWorldToUVScale(InvVolumeSize);
	XMVECTOR VolumeMinVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeMin);
	XMFLOAT3 VLMWorldToUVAdd;
	XMStoreFloat3(&VLMWorldToUVAdd, XMVectorMultiply(-VolumeMinVec, InvVolumeSizeVec));
	m_BasicEffect.SetVLMWorldToUVAdd(VLMWorldToUVAdd);


	ComPtr<ID3D11ShaderResourceView> SRV;
	CreateTexture3D(m_pd3dDevice.Get(), 
		m_pd3dImmediateContext.Get(), 
		vlmData.textureDimension.z, 
		vlmData.textureDimension.x, 
		vlmData.textureDimension.y, 
		vlmData.indirectionTexture.Format, 
		vlmData.indirectionTexture.data, 
		m_pTex3D.ReleaseAndGetAddressOf(),
		SRV.ReleaseAndGetAddressOf());
	m_BasicEffect.SetTexture3D(SRV.Get());

	CreateTexture3D(m_pd3dDevice.Get(),
		m_pd3dImmediateContext.Get(),
		vlmData.brickDataDimension.z,
		vlmData.brickDataDimension.x,
		vlmData.brickDataDimension.y,
		vlmData.brickData.AmbientVector.Format,
		vlmData.brickData.AmbientVector.data,
		m_pTex3D.ReleaseAndGetAddressOf(),
		SRV.ReleaseAndGetAddressOf());
	m_BasicEffect.SetTexture3D(SRV.Get());

	for (int i = 0; i < 6; i++) {
		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[i].Format,
			vlmData.brickData.SHCoefficients[i].data,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());
	}

	m_BasicEffect.SetSHUsed(m_UseSH);
	return true;
}


bool TestVLM::InitResource() {
	const XMFLOAT3& VolumeSize = m_Importer.VLMSetting.VolumeSize;
	const XMFLOAT3& VolumeMin = m_Importer.VLMSetting.VolumeMin;
	XMVECTOR VolumeMinVec = XMLoadFloat3(&VolumeMin);
	XMVECTOR VolumeSizeVec = XMLoadFloat3(&VolumeSize);
	XMVECTOR BoundCentreVec = XMVectorAdd(VolumeMinVec, XMVectorDivide(VolumeSizeVec, XMVectorSet(2.0f, 2.0f, 2.0f, 2.0f)));
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

	// 获取房屋包围盒
	/*XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);*/
	XMMATRIX S = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	BoundingBox houseBox = m_Sponza.GetLocalBoundingBox();
	houseBox.Transform(houseBox, S);

	Transform& houseTransform = m_Sponza.GetTransform();
	houseTransform.SetScale(1.0f, 1.0f, 1.0f);
	houseTransform.SetPosition(0.0f, 0.0f, 0.0f);


	/*******************************
		初始化摄像机, 默认第三人称视角
	********************************/
	m_CameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FPSCamera>(new FPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 10000.0f);
	camera->LookTo(XMFLOAT3(0.0f, 300.0f, 0.0f), XMFLOAT3(1, 0, 0), XMFLOAT3(0.0f, 1.0f, 0.0f));

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
	dirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	m_BasicEffect.SetDirLightNums(1);
	for (int i = 0; i < 11; i++) {
		PointLight pointLight;
		pointLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
		pointLight.range = 500.0f;
		m_PointLightArray.push_back(pointLight);
	}
	m_PointLightArray[0].position = XMFLOAT3(-1200.0f, 150.0f, -420.0f);
	m_PointLightArray[1].position = XMFLOAT3(-1200.0f, 150.0f, 435.0f);
	m_PointLightArray[1].diffuse = XMFLOAT4(0.0f, 0.0f, 0.8f, 1.0f);
	m_PointLightArray[2].position = XMFLOAT3(0.0f, 150.0f, -420.0f);
	m_PointLightArray[3].position = XMFLOAT3(0.0f, 150.0f, 430.0f);
	m_PointLightArray[4].position = XMFLOAT3(1100.0f, 150.0f, -410.0f);
	m_PointLightArray[4].diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_PointLightArray[5].position = XMFLOAT3(1100.0f, 150.0f, 0.0f);
	m_PointLightArray[6].position = XMFLOAT3(1100.0f, 150.0f, 435.0f);
	m_PointLightArray[7].position = XMFLOAT3(494.0f, 150.0f, -145.0f);
	m_PointLightArray[8].position = XMFLOAT3(494.0f, 150.0f, 215.0f);
	m_PointLightArray[9].position = XMFLOAT3(-620.0f, 150.0f, 210.0f);
	m_PointLightArray[10].position = XMFLOAT3(-620.0f, 150.0f, -140.0f);
	for (int i = 0; i < m_PointLightArray.size(); i++) {
		m_BasicEffect.SetPointLight(i, m_PointLightArray[i]);
	}
	m_BasicEffect.SetPointLightNums(m_PointLightArray.size());

	m_BasicEffect.SetSpotLightNums(0);
	/*
		动态物体
	*/
	for (int i = 0; i < 3; i++) {
		m_Spheres.push_back(GameObject());
		m_SpheresDirection.push_back(1);
		GameObject& sphere = m_Spheres.back();
		sphere.SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateSphere(40.0f)));
	}
	m_Spheres[0].GetTransform().SetPosition(0, 240, 0);
	m_Spheres[1].GetTransform().SetPosition(1000, 240, 450);
	m_Spheres[2].GetTransform().SetPosition(-1000, 240, -400);


	VisualizeVLM();

	/*
		设置阴影矩阵
		稍微高一点位置以显示阴影
	*/
	//m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
	//	XMVectorSet(pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f)));

	return true;
}

