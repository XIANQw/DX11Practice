#include "Ex10Camera.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

using namespace DirectX;


Ex10Camera::GameObject::GameObject() :
	m_VertexStride(),
	m_IndexCount() {
}



// 获取物体变换
Transform& Ex10Camera::GameObject::GetTransform() {
	return m_Transform;
}

// 获取物体变换
const Transform& Ex10Camera::GameObject::GetTransform() const {
	return m_Transform;
}

/*
	用于构造顶点和索引，resetMesh被移到这里
*/
template<class VertexType, class IndexType>
void Ex10Camera::GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData) {
	// 1.清空顶点和索引缓冲区
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	/*****************
	 * 2 顶点缓冲区   *
	******************/
	// 2.1 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	m_VertexStride = sizeof(VertexType);
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 2.2 设定用于初始化的数据并在Device上建立缓冲区
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf()));

	/**************
	 *3 索引缓冲区 *
	**************/
	m_IndexCount = (UINT)meshData.indexVec.size();
	// 3.1 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = (UINT)meshData.indexVec.size() * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 3.2 建立索引缓冲区
	initData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf()));

}


void Ex10Camera::GameObject::SetTexture(ID3D11ShaderResourceView* texture) {
	m_pTexture = texture;
}


void Ex10Camera::GameObject::Draw(ID3D11DeviceContext* deviceContext) {
	// 在上下文装配顶点缓冲区
	UINT stride = m_VertexStride;
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	// 在上下文上装配索引缓冲区
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 获取之前绑定到VS上的常量缓冲区并修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBChangesEveryDrawing cbDrawing;

	// Transpose
	XMMATRIX W = m_Transform.GetLocalToWorldMatrixXM();
	cbDrawing.world = XMMatrixTranspose(W);
	cbDrawing.worldInvTranspose = XMMatrixInverse(nullptr, W); // W 已经是世界矩阵的转置矩阵

	// 更新Context的drawing常量缓冲
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}


Ex10Camera::Ex10Camera(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_CameraMode(CameraMode::FPS),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely() {
}


Ex10Camera::~Ex10Camera() {

}


bool Ex10Camera::Init() {
	if (!D3DApp::Init()) return false;
	if (!InitEffect()) return false;
	if (!InitResource()) return false;

	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(Mouse::MODE_RELATIVE);

	return true;
}


void Ex10Camera::UpdateScene(float dt)
{
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();
	Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	Transform& woodCrateTransform = m_WoodCrate.GetTransform();

	auto cam1st = std::dynamic_pointer_cast<FPSCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<TPSCamera>(m_pCamera);

	if (m_CameraMode == CameraMode::FPS || m_CameraMode == CameraMode::Free) {
		// FPS mode
		if (keyState.IsKeyDown(Keyboard::W)) {
			if (m_CameraMode == CameraMode::FPS) {
				cam1st->Walk(dt * 5.0f);
			}
			else {
				cam1st->MoveForward(dt * 5.0f);
			}
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			if (m_CameraMode == CameraMode::FPS) {
				cam1st->Walk(-dt * 5.0f);
			}
			else {
				cam1st->MoveForward(-dt * 5.0f);
			}
		}
		if (keyState.IsKeyDown(Keyboard::D)) cam1st->Strafe(dt * 5.0f);
		if (keyState.IsKeyDown(Keyboard::A)) cam1st->Strafe(-dt * 5.0f);
		
		// 限制摄像机在固定范围内 x(-8.9, 8.9), z(-8.9, 8.9), y(0, 8.9) y不能为负
		XMFLOAT3 adjustPos;
		// XMVectorClamp(V, min, max) 将V的每个分量限定在[min, max]范围
		XMStoreFloat3(&adjustPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		cam1st->SetPosition(adjustPos);

		// 只有第一人称才是摄像机和箱子都移动
		if (m_CameraMode == CameraMode::FPS) woodCrateTransform.SetPosition(adjustPos);

		if (mouseState.positionMode == Mouse::MODE_RELATIVE) {
			cam1st->Pitch(mouseState.y * dt * 2.5f);
			cam1st->RotateY(mouseState.x * dt * 2.5f);
		}
	}
	else if (m_CameraMode==CameraMode::Observe) {
		// 更新摄像机目标位置
		cam3rd->SetTarget(woodCrateTransform.GetPosition());
		// 摄像机绕目标旋转
		cam3rd->RotateX(mouseState.y * dt * 2.5f);
		cam3rd->RotateY(mouseState.x * dt * 2.5f);
		cam3rd->Approach(mouseState.scrollWheelValue / 120 * 1.0f);
	}
	else if (m_CameraMode == CameraMode::TPS) {
		if (keyState.IsKeyDown(Keyboard::W)) {
			woodCrateTransform.Translate(woodCrateTransform.GetForwardAxis(), dt * 5.0f);
		}
		if (keyState.IsKeyDown(Keyboard::S)) {
			woodCrateTransform.Translate(woodCrateTransform.GetForwardAxis(), -dt * 5.0f);
		}
		if (keyState.IsKeyDown(Keyboard::D)) 
			woodCrateTransform.Translate(woodCrateTransform.GetRightAxis(), dt * 5.0f);
		if (keyState.IsKeyDown(Keyboard::A))
			woodCrateTransform.Translate(woodCrateTransform.GetRightAxis(), -dt * 5.0f);
		cam3rd->SetTarget(woodCrateTransform.GetPosition());
		// 摄像机绕目标旋转
		cam3rd->RotateX(mouseState.y * dt * 2.5f);
		cam3rd->RotateY(mouseState.x * dt * 2.5f);
		cam3rd->Approach(mouseState.scrollWheelValue / 120 * 1.0f);
	}

	// 更新观察矩阵
	XMStoreFloat4(&m_CBFrame.eysPos, m_pCamera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());

	// 重置滚轮
	m_pMouse->ResetScrollWheelValue();

	// 切换到FPS模式
	if (keyState.IsKeyDown(Keyboard::D1) && m_CameraMode != CameraMode::FPS) {
		if (!cam1st) {
			cam1st.reset(new FPSCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}

		cam1st->LookTo(woodCrateTransform.GetPosition(), 
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_CameraMode = CameraMode::FPS;
	}
	// 切换到TPS模式
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::Observe) {
		if (!cam3rd) {
			cam3rd.reset(new TPSCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = woodCrateTransform.GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::Observe;
	}
	
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3) && m_CameraMode != CameraMode::Free) {
		if (!cam1st) {
			cam1st.reset(new FPSCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}
		XMFLOAT3 pos = woodCrateTransform.GetPosition();
		XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		pos.y += 3;
		cam1st->LookTo(pos, to, up);
		m_CameraMode = CameraMode::Free;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D4) && m_CameraMode != CameraMode::TPS) {
		if (!cam3rd) {
			cam3rd.reset(new TPSCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = woodCrateTransform.GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistMinMax(3.0f, 20.0f);

		m_CameraMode = CameraMode::TPS;
	}

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape)) {
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	}

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

}


void Ex10Camera::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_WoodCrate.Draw(m_pd3dImmediateContext.Get());
	m_Floor.Draw(m_pd3dImmediateContext.Get());
	for (int i = 0; i < m_Walls.size(); i++) {
		m_Walls[i].Draw(m_pd3dImmediateContext.Get());
	}

	HR(m_pSwapChain->Present(0, 0));
}


void Ex10Camera::OnResize() {
	D3DApp::OnResize();

	if (m_pCamera != nullptr) {
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

		// 更新Context的OnResize常量缓冲
		D3D11_MAPPED_SUBRESOURCE mappedRes;
		HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
		memcpy_s(mappedRes.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
		m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);
	}
}


bool Ex10Camera::InitEffect()
{
	ComPtr<ID3DBlob> blob;
	// 创建顶点着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_2D.cso", L"HLSL\\Basic_VS_2D.hlsl", "VS_2D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));
	// 创建顶点布局(2D)
	HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));
	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
	// 创建顶点布局(3D)
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

	// 创建像素着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_2D.cso", L"HLSL\\Basic_PS_2D.hlsl", "PS_2D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));
	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));

	return true;
}


bool Ex10Camera::InitResource()
{

	/*******************
		初始化纹理
	********************/
	ComPtr<ID3D11ShaderResourceView> texture;

	// 读取木箱
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodCrate.SetBuffer<VertexPosNormalTex, DWORD>(m_pd3dDevice.Get(), Geometry::CreateBox());
	m_WoodCrate.SetTexture(texture.Get());

	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.GetAddressOf()));
	m_Floor.SetBuffer<VertexPosNormalTex, DWORD>(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	/*************************************
						g_Walls[0]
					初始化墙壁 (0, 3, 10)
				   ________________
				 / |             / |
				/  |		    /  |
			   /___|___________/   |
			   |   |_ _ _ _ _ _|_ _|
			   |  /		       |  / g_Walls[1]
   (-10, 3, 0) | /			   | / (10, 3, 0)
   g_Walls[3]  |/______________|/
				  (0, 3, -10)
				 width g_Walls[2]
	*************************************/
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.GetAddressOf()));
	for (int i = 0; i < m_Walls.size(); i++) {
		m_Walls[i].SetBuffer<VertexPosNormalTex, DWORD>(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		Transform & transform = m_Walls[i].GetTransform();
		transform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * i, 0.0f);
		transform.SetPosition(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
		m_Walls[i].SetTexture(texture.Get());
	}

	// 创建并填充采样器状态 
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));

	/**************************
	* 设置并初始化常量缓冲区描述 *
	***************************/
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建4级常量缓冲区
	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[2].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesRarely);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[3].GetAddressOf()));

	// 初始化Frame常量缓冲区
	m_CameraMode = CameraMode::FPS;
	auto camera = std::shared_ptr<FPSCamera>(new FPSCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->LookAt(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// 初始化OnResize常量区
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());


	// 更新Context的OnResize常量缓冲
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(m_CBOnResize), &m_CBOnResize, sizeof(m_CBOnResize));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

	/******************
	* 初始化默认光照	  *
	*******************/
	// 环境光
	m_CBRarely.dirLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// 灯光
	m_CBRarely.pointLight[0].position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_CBRarely.pointLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_CBRarely.pointLight[0].range = 25.0f;
	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;
	// 初始化材质
	m_CBRarely.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.material.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_CBRarely.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 50.0f);

	// 更新Context的Rarely常量缓冲
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(m_CBRarely), &m_CBRarely, sizeof(m_CBRarely));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);



	/***************
	* 绑定资源到管线 *
	****************/

	// 设置图元类型
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());

	// 绑定3DVS到管线
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
	// 绑定3DPS着色器到管道
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);

	// CBDrawing 常量缓冲区对应HLSL寄存于b0 register
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	// CBFrame 常量缓冲区对应HLSL寄存于b1 register
	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	// CBOnResize 常量缓冲区对应HLSL寄存于b2 register
	m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());

	// CBRarely 常量缓冲区对应HLSL寄存于b3 register
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());



	/**************                  设置着色器资源                 *********************
	void ID3D11DeviceContext::PSSetShaderResources(
		UINT StartSlot,	// [In]起始槽索引，对应HLSL的register(t*)
		UINT NumViews,	// [In]着色器资源视图数目
		ID3D11ShaderResourceView * const *ppShaderResourceViews	// [In]着色器资源视图数组
	);
	***********************************************************************************/
	m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());


	return true;
}


