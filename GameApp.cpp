#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC GameApp::Vertex::inputlayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


GameApp::GameApp(HINSTANCE hInstance) :
	D3DApp(hInstance), m_CBuffer() {

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

	XMMATRIX scalling = XMMatrixScaling(objSize, objSize, objSize);

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
	// 世界空间变换，旋转和缩放
	m_CBuffer.world = XMMatrixTranspose(XMMatrixRotationY(theta) * XMMatrixRotationX(phi) * scalling);
	
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy_s(mappedRes.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);

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


	//创建并绑定inputlayout
	HR(m_pd3dDevice->CreateInputLayout(Vertex::inputlayout, ARRAYSIZE(Vertex::inputlayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

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
	Vertex vertices[] =
	{
		// cube
		{ XMFLOAT3(-2.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-2.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		
		// 锥
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(3.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(3.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(2.0f, 1.0f, 0), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	
	};

	// 建立顶点缓冲区
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 设定用于初始化的数据
	D3D11_SUBRESOURCE_DATA vinitData;
	ZeroMemory(&vinitData, sizeof(vinitData));
	vinitData.pSysMem = vertices;
	HR(m_pd3dDevice->CreateBuffer(&vbd, &vinitData, m_pVertexBuffer.GetAddressOf()));

	DWORD indices[] = {
		// 正面
		0, 1, 2,
		2, 3, 0,
		// 左面
		4, 5, 1,
		1, 0, 4,
		// 顶面
		1, 5, 6,
		6, 2, 1,
		// 背面
		7, 6, 5,
		5, 4, 7,
		// 右面
		3, 2, 6,
		6, 7, 3,
		// 底面
		4, 0, 3,
		3, 7, 4,

		// 锥
		// 底面 逆
		8,9,10,
		9,11,10,
		// 正面 顺
		9,8,12,
		//右面 顺
		11,9,12,
		//左面 逆
		10,12,8,
		//背面 逆
		10,11,12
	};
	// 建立索引缓冲区
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	vinitData.pSysMem = indices;
	HR(m_pd3dDevice->CreateBuffer(&ibd, &vinitData, m_pIndexBuffer.GetAddressOf()));
	// 绑定索引缓冲区到管道
	m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 建立常量缓冲区
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

	m_CBuffer.world = XMMatrixIdentity();
	m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));


	// 绑定顶点缓冲区到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);



	// 绑定常量缓冲区
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

	// 设置图元类型
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//绑定着色器到管道
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);


	return true;
}
