#pragma once

#include <wrl/client.h>
#include <string>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <string>
#include <memory>
#include <d3d11.h>
#include <sstream>

#include "GameTimer.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "DXTrace.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")


class D3DApp
{
public:
	D3DApp(HINSTANCE hinstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;	// 获取APP实例的句柄
	HWND MainWnd() const;		// 获取主窗口句柄
	float AspectRatio() const;	// 获取屏幕宽高比

	int Run();					// D3D 程序的主循环
	
	/*
		以下都是D3DApp的具体实例方法
	*/
	virtual bool Init();					// 总的初始化
	virtual void OnResize();				// 当窗口改变时的初始化
	virtual void UpdateScene(float dt) = 0;	// D3DApp实例用来更新每一帧
	virtual void DrawScene() = 0;			// D3DApp实例用来绘制每一帧
	// 窗口的消息回调函数
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	bool InitMainWindow();					// 窗口初始化
	bool InitDirect3D();					// 初始化D3D资源

	void CalculateFrameStats();				// 计算每秒帧数并显示在窗口

protected:

	HINSTANCE m_hAppInst;					// App实例句柄
	HWND m_hMainWnd;						// 主窗口句柄
	bool m_AppPaused;						// 应用是否暂停
	bool m_Minimized;						// 应用是否最小化
	bool m_Maximized;						// 应用是否最大化
	bool m_Resizing;						// 窗口大小是否变化
	bool m_Enable4xMsaa;					// 是否开启4x多重采样
	UINT m_4xMsaaQuality;					// MSAA支持的质量等级

	GameTimer m_Timer;						// 计时器

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	std::unique_ptr<DirectX::Mouse> m_pMouse;
	std::unique_ptr<DirectX::Keyboard> m_pKeyboard;
	// D3D11
	ComPtr<ID3D11Device> m_pd3dDevice;					// D3D11 设备
	ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;	// D3D11 设备上下文
	ComPtr<IDXGISwapChain> m_pSwapChain;				// D3D11 交换链

	// Resource
	ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;		// 深度模板缓冲区
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;	// 渲染目标视图
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;	// 深度模板视图
	D3D11_VIEWPORT m_ScreenViewport;					// 视口

	// 子类应该在构造函数设置的初始参数
	std::wstring m_MainWndTitle;						// 主窗口标题
	int m_ClientWidth;									// 视口宽度
	int m_ClientHeight;									// 视口高度

};

