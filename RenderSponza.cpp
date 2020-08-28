#include "RenderSponza.h"

using namespace DirectX;

RenderSponza::RenderSponza(HINSTANCE hInstance)
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
	m_SHMode(2),
	m_isVisulizeVLM(false),
	m_SphereSpeed(200),
	m_Text(L""),
	m_SHRepositoies{ L"50_metal", L"200", L"150", L"100", L"50" },
	m_SHFileIndex(0), m_isControlObj(false), m_TargetDistance(100){
}


RenderSponza::~RenderSponza() {

}


bool RenderSponza::Init() {

	if (!D3DApp::Init()) return false;
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_BasicEffect.SetVSShader3D(m_pd3dDevice.Get(), L"HLSL\\RenderSponza_VS3D.hlsl")) return false;
	if (!m_BasicEffect.SetPSShader3D(m_pd3dDevice.Get(), L"HLSL\\RenderSponza_PS3D.hlsl")) return false;
	if (!m_BasicEffect.SetInstanceVS(m_pd3dDevice.Get(), L"HLSL\\InstancesVertexShader.hlsl")) return false;
	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get())) return false;

	if (!InitVLM()) return false;
	if (!InitResource()) return false;

	m_BasicEffect.SetSHMode(m_SHMode);

	m_BasicEffect.SetDebugName();
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::Mode::MODE_RELATIVE);

	return true;
}


void RenderSponza::OnResize() {
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


/*
	每帧都会调用, 用于更新场景中各种变量
*/
void RenderSponza::UpdateScene(float dt) {
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	auto cam1st = std::dynamic_pointer_cast<FPSCamera>(m_pCamera);
	auto& sampleTrans = m_Sample.GetTransform();
	XMFLOAT3 curPos;
	if (m_CameraMode == CameraMode::Free) {
		// FPS mode
		if (keyState.IsKeyDown(Keyboard::W)) {
			cam1st->MoveForward(dt * m_Speed);
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			cam1st->MoveForward(-dt * m_Speed);
		}
		if (keyState.IsKeyDown(Keyboard::D)) {
			cam1st->Strafe(dt * m_Speed);
		}
		if (keyState.IsKeyDown(Keyboard::A)) {
			cam1st->Strafe(-dt * m_Speed);
		}

		// 限制摄像机在固定范围内
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMVECTOR VolumeMinVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeMin);
		XMVECTOR VolumeSizeVec = XMLoadFloat3(&m_Importer.VLMSetting.VolumeSize);
		XMVECTOR VolumeMaxVec = XMVectorAdd(VolumeMinVec, VolumeSizeVec);
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(), VolumeMinVec, VolumeMaxVec));

		cam1st->SetPosition(adjustPos);
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
		if (m_isControlObj) {
			m_TargetDistance += 0.05 * mouseState.scrollWheelValue;
			XMFLOAT3 forward = cam1st->GetLookAxis();
			sampleTrans.SetPosition(adjustPos.x + m_TargetDistance * forward.x, adjustPos.y + m_TargetDistance * forward.y, adjustPos.z + m_TargetDistance * forward.z);
		}
		BoundingBox houseBox = m_Sponza.GetBoundingBox();

		for (int i = 0; i < m_DynamicTransform.size(); i++) {
			auto& trans = m_DynamicTransform[i];
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
		if (m_SHFileIndex < m_SHRepositoies.size() - 1) {
			m_SHFileIndex += 1;
			m_BasicEffect.ClearTexture3D();
			InitVLM();
			VisualizeVLM();
		}
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::M)) {
		if (m_SHFileIndex > 0) {
			m_SHFileIndex -= 1;
			m_BasicEffect.ClearTexture3D();
			InitVLM();
			VisualizeVLM();
		}
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::L)) {
		m_BasicEffect.useNormalmap = !m_BasicEffect.useNormalmap;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::K)) {
		m_isControlObj = !m_isControlObj;
		XMFLOAT3 samplePos = m_Sample.GetTransform().GetPosition();
		cam1st->SetPosition(samplePos.x, samplePos.y + 100, samplePos.z - 100);
		cam1st->LookAt(cam1st->GetPosition(), samplePos, XMFLOAT3(0.0f, 1.0f, 0.0f));
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::H)) {
		m_ShowHelp = !m_ShowHelp;
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
	if (m_ShowHelp) {
		_snwprintf_s(m_Text, ARRAYSIZE(m_Text), ARRAYSIZE(m_Text) - 1, L"控制移动:WASD, VLM可视化:V, 线框模式:R, 控制摄像机速度:UY, 球谐光照开关:F, 纹理贴图开关:T, 阴影贴图开关:L \
控制小球:K, 增加DetailCellSize:N, 减少DetailCellSize:M, 方向光开关:1, 点光开关:2, 二阶球谐VS计算:3, 二阶球谐PS计算:4, 三阶球谐:5, 控制动态球速度:数字键盘+-");
	}
	else {
		_snwprintf_s(m_Text, ARRAYSIZE(m_Text), ARRAYSIZE(m_Text) - 1, L"SHMode=%d, 方向光开关=%d, 点光开关=%d, 位置(%.1f,%.1f,%.1f), \
VLM可视化=%d, 相机速度=%d, 动态球速度=%d, 阴影贴图=%d, \
detailCellSize=%s, VolumeSize(%.1f, %.1f, %.1f), 控制小球模式=%d, bricksNum=%d, IndirectionTextureMemory=%.1fKB, VLMTextureMemory=%.1fKB，按H查看按钮事件",
			m_SHMode, m_UseDirLight, m_UsePointLight, cam1st->GetPosition().x, cam1st->GetPosition().y, cam1st->GetPosition().z,
			m_isVisulizeVLM, m_Speed, m_SphereSpeed, m_BasicEffect.useNormalmap,
			m_SHRepositoies[m_SHFileIndex],
			m_Importer.VLMSetting.VolumeSize.x, m_Importer.VLMSetting.VolumeSize.y, m_Importer.VLMSetting.VolumeSize.z, m_isControlObj, m_Importer.m_BricksNum, m_Importer.vlmData.indirectionTexture.data.size() / 1024.0f,
			m_Importer.vlmData.brickData.AmbientVector.data.size() / 1024.0f * 7.0f);
	}

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape)) {
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}
}

/*
	每帧都会调用, 用于绘制画面
*/
void RenderSponza::DrawScene() {
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&m_BackGroundColor));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	{
		/************************
		1. 绘制场景
		*************************/
		if (m_IsWireframeMode) {
			m_BasicEffect.SetWireFrameWode(m_pd3dImmediateContext.Get());
			m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

			// 显示Volume的边界
			m_Box.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		}
		else {
			m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
			m_Sponza.SetMaterial(m_Material);
			m_Sponza.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
		}
	}


	// Draw observe Obj
	m_BasicEffect.SetTextureUsed(false);
	m_Sample.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// Draw dynamic obj
	m_BasicEffect.SetRenderInstanceDefault(m_pd3dImmediateContext.Get());
	m_Sample.DrawInstance(m_pd3dImmediateContext.Get(), m_BasicEffect, m_DynamicTransform);

	if (m_isVisulizeVLM) {
		for (INT32 depth = 0; depth < m_TransformData.size(); depth++)
			m_Sample.DrawInstance(m_pd3dImmediateContext.Get(), m_BasicEffect, m_TransformData[depth]);
	}

	m_BasicEffect.SetTextureUsed(m_UseTexture);
	WriteInformation(std::wstring(m_Text));

	HR(m_pSwapChain->Present(0, 0));

}

/*
	将indirectionTexturePosition 转成 WorldPosition
*/
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

void RenderSponza::VisualizeVLM() {

	INT32 SampleIndex = 0;
	m_TransformData.resize(m_Importer.m_BricksByDepth.size());
	for (INT32 depth = 0; depth < m_Importer.m_BricksByDepth.size(); depth++) {
		auto& TransformDataAtDepth = m_TransformData[depth];
		TransformDataAtDepth.resize(m_Importer.m_BricksByDepth[depth].size());

		for (INT32 index = 0; index < m_Importer.m_BricksByDepth[depth].size(); index++) {
			const auto& brick = m_Importer.m_BricksByDepth[depth][index];
			TransformDataAtDepth[index].SetScale(XMFLOAT3(3 - depth, 3 - depth, 3 - depth));
			TransformDataAtDepth[index].SetPosition(IndTexPosToWorldPos(brick.IndirectionTexturePosition, m_Importer.VLMSetting, m_Importer.vlmData.textureDimension));
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

bool RenderSponza::InitVLM() {

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
		indTexRepo, AmbientVecRepo, SH0Repo, SH1Repo, SH2Repo, SH3Repo, SH4Repo, SH5Repo);
	/*{

			// Should be deleted after test !!!!!!!

			m_Importer.ImportFile(L"D:\\brickDataByDepth", L"D:\\vlmSetting",
				L"D:\\indirectionTexture", L"D:\\AmbientVector",
				L"D:\\SH0", L"D:\\SH1", L"D:\\SH2", L"D:\\SH3",
				L"D:\\SH4", L"D:\\SH5");
		}*/

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

	/*
	{
		//Test part, should be deleted

		std::ifstream SH0Phase1Importer(L"D:\\SH0Phase1", std::ios::in | std::ios::binary);
		std::ifstream SH0Phase2Importer(L"D:\\SH0Phase2", std::ios::in | std::ios::binary);
		std::ifstream SH0Phase3Importer(L"D:\\SH0Phase3", std::ios::in | std::ios::binary);
		std::ifstream SH0Phase4Importer(L"D:\\SH0Phase4", std::ios::in | std::ios::binary);
		std::ifstream SH0Phase5Importer(L"D:\\SH0Phase5", std::ios::in | std::ios::binary);
		std::vector<UINT8>SH0Phase1(vlmData.brickData.SHCoefficients[0].data.size());
		std::vector<UINT8>SH0Phase2(vlmData.brickData.SHCoefficients[0].data.size());
		std::vector<UINT8>SH0Phase3(vlmData.brickData.SHCoefficients[0].data.size());
		std::vector<UINT8>SH0Phase4(vlmData.brickData.SHCoefficients[0].data.size());
		std::vector<UINT8>SH0Phase5(vlmData.brickData.SHCoefficients[0].data.size());
		SH0Phase1Importer.read(reinterpret_cast<char*>(SH0Phase1.data()), vlmData.brickData.SHCoefficients[0].data.size());
		SH0Phase2Importer.read(reinterpret_cast<char*>(SH0Phase2.data()), vlmData.brickData.SHCoefficients[0].data.size());
		SH0Phase3Importer.read(reinterpret_cast<char*>(SH0Phase3.data()), vlmData.brickData.SHCoefficients[0].data.size());
		SH0Phase4Importer.read(reinterpret_cast<char*>(SH0Phase4.data()), vlmData.brickData.SHCoefficients[0].data.size());
		SH0Phase5Importer.read(reinterpret_cast<char*>(SH0Phase5.data()), vlmData.brickData.SHCoefficients[0].data.size());
		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[0].Format,
			SH0Phase1,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());

		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[0].Format,
			SH0Phase2,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());

		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[0].Format,
			SH0Phase3,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());

		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[0].Format,
			SH0Phase4,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());

		CreateTexture3D(m_pd3dDevice.Get(),
			m_pd3dImmediateContext.Get(),
			vlmData.brickDataDimension.z,
			vlmData.brickDataDimension.x,
			vlmData.brickDataDimension.y,
			vlmData.brickData.SHCoefficients[0].Format,
			SH0Phase5,
			m_pTex3D.ReleaseAndGetAddressOf(),
			SRV.ReleaseAndGetAddressOf());
		m_BasicEffect.SetTexture3D(SRV.Get());
	}
	*/

	return true;
}


bool RenderSponza::InitResource() {
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
	for (int i = 0; i < 22; i++) {
		PointLight pointLight;
		pointLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
		pointLight.range = 500.0f;
		m_PointLightArray.push_back(pointLight);
	}
	// 1st floor
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
	// 2nd floor
	m_PointLightArray[11].position = XMFLOAT3(-1250.0f, 570.0f, 0.0f);
	m_PointLightArray[12].position = XMFLOAT3(-1150.0f, 570.0f, 480.0f);
	m_PointLightArray[13].position = XMFLOAT3(-1150.0f, 470.0f, -390.0f);
	m_PointLightArray[14].position = XMFLOAT3(0.0f, 570.0f, 480.0f);
	m_PointLightArray[15].position = XMFLOAT3(1100.0f, 570.0f, 480.0f);
	m_PointLightArray[16].position = XMFLOAT3(0.0f, 570.0f, -370.0f);
	m_PointLightArray[17].position = XMFLOAT3(1100.0f, 570.0f, -360.0f);
	m_PointLightArray[18].position = XMFLOAT3(1150.0f, 570.0f, 30.0f);
	// sky
	m_PointLightArray[19].position = XMFLOAT3(540.0f, 850.0f, 0.0f);
	m_PointLightArray[19].diffuse = XMFLOAT4(0.0f, 1.0f, 0.3f, 1.0f);
	m_PointLightArray[20].position = XMFLOAT3(-90.0f, 850.0f, 0.0f);
	m_PointLightArray[20].diffuse = XMFLOAT4(1.0f, 0.2f, 0.0f, 1.0f);
	m_PointLightArray[21].position = XMFLOAT3(-750.0f, 850.0f, 0.0f);
	m_PointLightArray[21].diffuse = XMFLOAT4(0.0f, 0.12f, 1.0f, 1.0f);

	for (int i = 0; i < m_PointLightArray.size(); i++) {
		m_BasicEffect.SetPointLight(i, m_PointLightArray[i]);
	}
	m_BasicEffect.SetPointLightNums(m_PointLightArray.size());

	m_BasicEffect.SetSpotLightNums(0);
	/*
		动态物体
	*/
	m_DynamicTransform.resize(3);
	m_SpheresDirection.resize(3, 1);
	for (int i = 0; i < 3; i++) {
		m_DynamicTransform[i].SetScale(4, 4, 4);
	}
	m_DynamicTransform[0].SetPosition(0, 240, 0);
	m_DynamicTransform[1].SetPosition(1000, 240, 450);
	m_DynamicTransform[2].SetPosition(-1000, 240, -400);

	// OberveObj
	m_Sample.SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateSphere(10.0f)));
	m_Sample.GetTransform().SetPosition(0, 800, 0);
	m_Sample.GetTransform().SetScale(3, 3, 3);
	m_Sample.SetMaterial(material);

	// Reflection plane
	m_plane.SetModel(Model(m_pd3dDevice.Get(), Geometry::CreateBox(100, 100, 10)));
	m_plane.GetTransform().SetPosition(1210.0f, 235.0f, -390.0f);
	m_Sample.SetMaterial(material);


	VisualizeVLM();

	/*
		设置阴影矩阵
		稍微高一点位置以显示阴影
	*/
	//m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
	//	XMVectorSet(pointLight.position.x, pointLight.position.y, pointLight.position.z, 1.0f)));

	return true;
}

