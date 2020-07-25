#include "d3dApp.h"

namespace {
	/*
		这是一个全局指针, 用于调用回调函数MsgProc
	*/
	D3DApp* g_pd3dApp = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hinstance) :
	m_hAppInst(hinstance),
	m_hMainWnd(nullptr),
	m_ClientWidth(1280),
	m_ClientHeight(720),
	m_AppPaused(false),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_Enable4xMsaa(true),
	m_4xMsaaQuality(0),
	m_MainWndTitle(L"Render something"),
	m_pd3dDevice(nullptr),
	m_pd3dImmediateContext(nullptr),
	m_pSwapChain(nullptr),
	m_pDepthStencilBuffer(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	g_pd3dApp = this;
}
D3DApp::~D3DApp() {
	if (m_pd3dImmediateContext) {
		m_pd3dImmediateContext->ClearState();
	}
}

HINSTANCE D3DApp::AppInst() const {
	return m_hAppInst;
}	

HWND D3DApp::MainWnd() const {
	return m_hMainWnd;
}		

float D3DApp::AspectRatio() const {
	return static_cast<float>(m_ClientWidth)/m_ClientHeight;
}

int D3DApp::Run() {
	MSG msg = { 0 };
	m_Timer.Reset();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_Timer.Tick();
			if (!m_AppPaused) {
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			}
			else {
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

bool D3DApp::Init() {
	m_pMouse = std::make_unique<DirectX::Mouse>();
	m_pKeyboard = std::make_unique<DirectX::Keyboard>();
	
	if (!InitMainWindow()) return false;
	if (!InitDirect3D()) return false;

	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	
	return true;
}

void D3DApp::OnResize() {
	assert(m_pd3dDevice);
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pDepthStencilBuffer.Reset();
	m_pRenderTargetView.Reset();
	m_pDepthStencilView.Reset();

	/*
		创建渲染目标视图: 1. 获取后备缓冲区 2. 将缓冲区绑定到视图上
	*/
	ComPtr<ID3D11Texture2D> backBuffer;
	m_pSwapChain->ResizeBuffers(1,
		m_ClientWidth,
		m_ClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(),
		nullptr, m_pRenderTargetView.GetAddressOf());

	D3D11_TEXTURE2D_DESC tex2DDesc;
	ZeroMemory(&tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
	tex2DDesc.Width = m_ClientWidth;
	tex2DDesc.Height = m_ClientHeight;
	tex2DDesc.MipLevels = 1;
	tex2DDesc.ArraySize = 1;
	tex2DDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (m_Enable4xMsaa) {
		tex2DDesc.SampleDesc.Count = 4;
		tex2DDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else {
		tex2DDesc.SampleDesc.Count = 1;
		tex2DDesc.SampleDesc.Quality = 0;
	}
	tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
	tex2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2DDesc.CPUAccessFlags = 0;
	tex2DDesc.MiscFlags = 0;
	// 创建深度模板缓冲区
	m_pd3dDevice->CreateTexture2D(
		&tex2DDesc, nullptr, m_pDepthStencilBuffer.GetAddressOf());
	// 创建深度模板视图
	m_pd3dDevice->CreateDepthStencilView(
		m_pDepthStencilBuffer.Get(), nullptr,
		m_pDepthStencilView.GetAddressOf());
	// 将渲染目标视图和深度模板视图绑定到渲染管线
	m_pd3dImmediateContext->OMSetRenderTargets(1,
		m_pRenderTargetView.GetAddressOf(),
		m_pDepthStencilView.Get());

	// 设置视口变换
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}


// 窗口的消息回调函数
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			m_AppPaused = true;
			m_Timer.Stop();
		}
		else {
			m_AppPaused = false;
			m_Timer.Start();
		}
		return 0;
	case WM_SIZE:
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		if (m_pd3dDevice) {
			if (wParam == SIZE_MINIMIZED) {
				m_AppPaused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED) {
				m_AppPaused = false;
				m_Minimized = false;
				m_Maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED) {
				if (m_Maximized) {
					m_AppPaused = false;
					m_Maximized = false;
					OnResize();
				}
				else if (m_Minimized) {
					m_AppPaused = false;
					m_Minimized = false;
					OnResize();
				}
				else if (m_Resizing) {
				}
				else {
					OnResize();
				}
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		m_AppPaused = true;
		m_Resizing = true;
		m_Timer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		m_AppPaused = false;
		m_Resizing = false;
		m_Timer.Start();
		OnResize();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;


		// mouse
	case WM_INPUT:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_XBUTTONDOWN:

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSEHOVER:
	case WM_MOUSEMOVE:
		m_pMouse->ProcessMessage(msg, wParam, lParam);
		return 0;
		// keyboard
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		m_pKeyboard->ProcessMessage(msg, wParam, lParam);
		return 0;
	case WM_ACTIVATEAPP:
		m_pMouse->ProcessMessage(msg, wParam, lParam);
		m_pKeyboard->ProcessMessage(msg, wParam, lParam);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



bool D3DApp::InitMainWindow() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndTitle.c_str(),
		WS_OVERLAPPEDWINDOW, (screenWidth - width) / 2, (screenHeight - height) / 2, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

bool D3DApp::InitDirect3D() {
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#if defined(DEBUG)|| defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	/*
		创建D3DDevice和DeviceContext
	*/
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDevice(nullptr,
		driverType,
		nullptr,
		createDeviceFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		m_pd3dDevice.GetAddressOf(),
		&featureLevel,
		m_pd3dImmediateContext.GetAddressOf());
	
	if (FAILED(hr)) {
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return false;
	}

	m_pd3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
	assert(m_4xMsaaQuality > 0);

	/*
		初始化DXGI交换链
	*/
	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
	ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr;
	
	HR(m_pd3dDevice.As(&dxgiDevice));
	HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), 
		reinterpret_cast<void**>(dxgiFactory1.GetAddressOf())));
	
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferDesc.Width = m_ClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_ClientHeight;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	
	if (m_Enable4xMsaa) {
		dxgiSwapChainDesc.SampleDesc.Count = 4;
		dxgiSwapChainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else {
		dxgiSwapChainDesc.SampleDesc.Count = 1;
		dxgiSwapChainDesc.SampleDesc.Quality = 0;
	}
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.OutputWindow = m_hMainWnd;
	dxgiSwapChainDesc.Windowed = true;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dxgiSwapChainDesc.Flags = 0;
	HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), 
		&dxgiSwapChainDesc, 
		m_pSwapChain.GetAddressOf()));
	
	dxgiFactory1->MakeWindowAssociation(m_hMainWnd, 
		DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
	
	OnResize();
}

void D3DApp::CalculateFrameStats()
{
	// 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndTitle << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


