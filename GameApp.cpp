#include "GameApp.h"

GameApp::GameApp(HINSTANCE hinstance):
	D3DApp(hinstance)
{
}

GameApp::~GameApp() {
}

bool GameApp::Init() {
	if (!D3DApp::Init()) {
		return false;
	}
	return true;
}


void GameApp::OnResize() {
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt) {

}

void GameApp::DrawScene() {
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(),
		reinterpret_cast<const float*>(&DirectX::Colors::Blue));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pSwapChain->Present(0, 0);
}
