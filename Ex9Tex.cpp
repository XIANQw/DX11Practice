#include "Ex9Tex.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
using namespace DirectX;


Ex9Tex::Ex9Tex(HINSTANCE hInstance)
	:D3DApp(hInstance),
	m_IndexCount(),
	m_CurrFrame(),
	m_CurrMode(ShowMode::WoodCrate),
	m_VSConstantBuffer(),
	m_PSConstantBuffer(),
	m_PointLight()
{
}

Ex9Tex::~Ex9Tex()
{
}

bool Ex9Tex::Init()
{
	if (!D3DApp::Init()) return false;
	if (!InitEffect()) return false;
	if (!InitResource()) return false;


	return true;
}

void Ex9Tex::OnResize()
{
	D3DApp::OnResize();
}

void Ex9Tex::UpdateScene(float dt)
{
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();
	Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();
	static float phi = 0.0f, theta = 0.0f, objSize = 1.0f;

	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);

	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1)) {
		m_CurrMode = ShowMode::WoodCrate;
		m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
		auto meshData = Geometry::CreateBox<VertexPosNormalTex>();
		ResetMesh(meshData);
		m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
		m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
		m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CurrMode != ShowMode::FireAnim) {
		m_CurrMode = ShowMode::FireAnim;
		m_CurrFrame = 0;
		m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout2D.Get());
		auto meshData = Geometry::Create2DShow();
		ResetMesh(meshData);
		m_pd3dImmediateContext->VSSetShader(m_pVertexShader2D.Get(), nullptr, 0);
		m_pd3dImmediateContext->PSSetShader(m_pPixelShader2D.Get(), nullptr, 0);
		m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pFireAnims[0].GetAddressOf());
	}

	if (m_CurrMode == ShowMode::WoodCrate) {
		int deltaScrollWhellValue = mouseState.scrollWheelValue - lastMouseState.scrollWheelValue;
		objSize += deltaScrollWhellValue * 0.0001f;
		XMMATRIX scaling = XMMatrixScaling(objSize, objSize, objSize);

		if (mouseState.leftButton == true && m_MouseTracker.leftButton == m_MouseTracker.HELD)
		{
			int dx = (mouseState.x - lastMouseState.x), dy = mouseState.y - lastMouseState.y;
			theta -= dx * 0.01f;
			phi -= dy * 0.01f;
		}
		if (keyState.IsKeyDown(Keyboard::W))
			phi += dt * 2;
		if (keyState.IsKeyDown(Keyboard::S))
			phi -= dt * 2;
		if (keyState.IsKeyDown(Keyboard::A))
			theta += dt * 2;
		if (keyState.IsKeyDown(Keyboard::D))
			theta -= dt * 2;
		// 世界空间变换: 缩放与旋转
		XMMATRIX transform = scaling * XMMatrixRotationY(theta) * XMMatrixRotationX(phi);
		m_VSConstantBuffer.world = XMMatrixTranspose(transform);
		// 转置逆矩阵
		m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, transform);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
		memcpy_s(mappedRes.pData, sizeof(m_VSConstantBuffer), &m_VSConstantBuffer, sizeof(m_VSConstantBuffer));
		m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);
	}
	else if (m_CurrMode == ShowMode::FireAnim) {
		// 规定fps60
		static float totDeltaTime = 0;
		totDeltaTime += dt;
		if (totDeltaTime > 1.0f / 60){
			totDeltaTime -= 1.0f / 60;
			m_CurrFrame = (m_CurrFrame + 1) % 120;
			m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pFireAnims[m_CurrFrame].GetAddressOf());
		}
	}



}

void Ex9Tex::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);
	HR(m_pSwapChain->Present(0, 0));
}

bool Ex9Tex::InitEffect()
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

bool Ex9Tex::InitResource()
{

	/**************************
	* 重置顶点缓冲区和索引缓冲区 *
	***************************/

	Geometry::MeshData<VertexPosNormalTex> meshdata = Geometry::CreateBox<VertexPosNormalTex>();
	ResetMesh(meshdata);

	/*******************
		初始化纹理
	********************/
	// 读取木箱
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pWoodCrate.GetAddressOf()));
	// 读取火焰
	m_pFireAnims.resize(120);
	WCHAR filename[40];
	for (int i = 1; i <= 120; ++i)
	{
		wsprintf(filename, L"Texture\\FireAnim\\Fire%03d.bmp", i);
		HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), filename, nullptr, m_pFireAnims[static_cast<size_t>(i) - 1].GetAddressOf()));
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

	/******************
	* 初始化默认光照	  *
	*******************/
	// 点光
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 25.0f;

	/**************************
	* 设置并初始化常量缓冲区描述 *
	***************************/
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建VS和PS常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

	// 初始化VS常量缓冲区
	m_VSConstantBuffer.world = XMMatrixIdentity();
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	// 初始化PS常量缓冲区
	// 初始化光照模型
	m_PSConstantBuffer.pointLight[0] = m_PointLight;
	m_PSConstantBuffer.numDirLight = 0;
	m_PSConstantBuffer.numPointLight = 1;
	m_PSConstantBuffer.numSpotLight = 0;
	// 初始化材质
	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	// 初始化摄像机位置
	m_PSConstantBuffer.eyePos = XMFLOAT4(0, 0, -5.0f, 0);

	// 更新Context的PS常量缓冲
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

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

	// VS 常量缓冲区对应HLSL寄存于b0 register
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	// PS常量缓冲区对应HLSL寄存于b1 register
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	

	/**************                  设置着色器资源                 *********************
	void ID3D11DeviceContext::PSSetShaderResources(
		UINT StartSlot,	// [In]起始槽索引，对应HLSL的register(t*)
		UINT NumViews,	// [In]着色器资源视图数目
		ID3D11ShaderResourceView * const *ppShaderResourceViews	// [In]着色器资源视图数组
	);
	***********************************************************************************/
	m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
	m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());


	return true;
}


template<class VertexType>
bool Ex9Tex::ResetMesh(const Geometry::MeshData<VertexType>& meshData) {
	// 1.清空顶点和索引缓冲区
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	/*****************
	 * 2 顶点缓冲区   *
	******************/
	// 2.1 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 2.2 设定用于初始化的数据并在Device上建立缓冲区
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf()));
	// 2.3 在上下文装配顶点缓冲区
	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);


	/**************
	 *3 索引缓冲区 *
	**************/
	m_IndexCount = (UINT)meshData.indexVec.size();
	// 3.1 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = (UINT)meshData.indexVec.size() * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 3.2 建立索引缓冲区
	initData.pSysMem = meshData.indexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf()));
	// 3.3 在上下文上装配索引缓冲区
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	return true;
}



