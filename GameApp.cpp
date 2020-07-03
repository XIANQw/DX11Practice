#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance) :
	D3DApp(hInstance), 
	m_IndexCount(), m_PSConstantBuffer(), m_VSConstantBuffer(), m_DirLight(), m_IsWireframeMode(false){

}

GameApp::~GameApp() {

}

bool GameApp::Init() {
	if (!D3DApp::Init()) {
		return false;
	}
	if (!InitEffect()) {
		return false;
	}
	if (!InitResource()) {
		return false;
	}

	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	return true;
}

void GameApp::OnResize() {
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt) {
	
	// 获取鼠标状态
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	// 获取键盘状态
	Keyboard::State keyState = m_pKeyboard->GetState();
	Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();
	static float phi = 0.0f, theta = 0.0f, objSize = 1.0f;
	
	m_MouseTracker.Update(mouseState);
	m_KeyboardTracker.Update(keyState);
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
	
	// 切换光栅化状态
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::S))
	{
		m_IsWireframeMode = !m_IsWireframeMode;
		m_pd3dImmediateContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
	}


	// 世界空间变换，旋转和缩放
	m_VSConstantBuffer.world = XMMatrixTranspose(XMMatrixRotationY(theta) * XMMatrixRotationX(phi) * scaling);
	
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffer[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(m_VSConstantBuffer), &m_VSConstantBuffer, sizeof(m_VSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer[0].Get(), 0);

	HR(m_pd3dImmediateContext->Map(m_pConstantBuffer[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(m_PSConstantBuffer), &m_PSConstantBuffer, sizeof(m_PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer[1].Get(), 0);

}

void GameApp::DrawScene() {
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pd3dImmediateContext->DrawIndexed(54, 0, 0);
	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect() {
	//创建顶点着色器
	ComPtr<ID3DBlob> blob;
	// 将HLSL文件转成二进制码
	HR(CreateShaderFromFile(L"HLSL\\VertexShader.cso", L"HLSL\\VertextShader.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	// 创建顶点输入布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	//创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\PixelShader.cso", L"HLSL\\PixelShader.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource() {
	// ******************
	// 设置立方体顶点
	//    5________ 6
	//    /|      /|
	//   /_|_____/ |
	//  1|4|_ _ 2|_|7     10________11
	//   | /     | /       /       /
	//   |/______|/       /_______/
	//  0       3        8       9
	
	/**************************
	* 重置顶点缓冲区和索引缓冲区 *
	***************************/

	Geometry::MeshData<VertexPosNormalColor> meshdata = Geometry::CreateBox<VertexPosNormalColor>();
	ResetMesh(meshdata);


	/******************
	* 初始化默认光照	  *
	*******************/
	// 平行光
	m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	// 点光

	// 聚光

	/************
	* 常量缓冲区 *
	*************/
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建VS和PS常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer[1].GetAddressOf()));
	
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
	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	m_PSConstantBuffer.dirLight = m_DirLight;
	m_PSConstantBuffer.eyePos = XMFLOAT4(0, 0, -0.5f, 0);

	// 更新Context的PS常量缓冲
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffer[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer[1].Get(), 0);

	/*****************
	* 初始化光栅化状态 *
	******************/
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

	/***************
	* 绑定资源到管线 *
	****************/

	// 设置图元类型
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

	// 绑定VS常量缓冲区到管线
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	// VS 常量缓冲区对应HLSL寄存于b0 register
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[0].GetAddressOf());

	// 绑定PS着色器到管道
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	// PS 常量缓冲区对应HLSL寄存于b1 register
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffer[1].GetAddressOf());


	return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData) {
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
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalColor);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 2.2 设定用于初始化的数据并在Device上建立缓冲区
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf()));
	// 2.3 在上下文装配顶点缓冲区
	UINT stride = sizeof(VertexPosNormalColor);
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
	ibd.ByteWidth = (UINT)meshData.indexVec.size()*sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 3.2 建立索引缓冲区
	initData.pSysMem = meshData.indexVec.data();
	HR(m_pd3dDevice->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf()));
	// 3.3 在上下文上装配索引缓冲区
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	return true;


}



