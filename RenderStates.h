﻿#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>


/******************************
*		RenderStates.h        *
*	该类保存了所有的渲染配置：   *
*	三种光栅化配置，            *
*	两种采样器配置，            *
*	三种混合状态，              *
*	五种深度模板状态            *
*******************************/


class RenderStates
{
public:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static bool IsInit();

	static void InitAll(ID3D11Device* device);
	// 使用ComPtr智能指针无需手工释放

public:
	static ComPtr<ID3D11RasterizerState> RSWireframe;		    // 光栅化器状态：线框模式
	static ComPtr<ID3D11RasterizerState> RSNoCull;			    // 光栅化器状态：无背面裁剪模式
	static ComPtr<ID3D11RasterizerState> RSCullClockWise;	    // 光栅化器状态：顺时针裁剪模式

	static ComPtr<ID3D11SamplerState> SSLinearWrap;			    // 采样器状态：线性过滤
	static ComPtr<ID3D11SamplerState> SSAnistropicWrap;		    // 采样器状态：各项异性过滤

	static ComPtr<ID3D11BlendState> BSNoColorWrite;		        // 混合状态：不写入颜色
	static ComPtr<ID3D11BlendState> BSTransparent;		        // 混合状态：透明混合
	static ComPtr<ID3D11BlendState> BSAlphaToCoverage;	        // 混合状态：Alpha-To-Coverage

	static ComPtr<ID3D11DepthStencilState> DSSWriteStencil;		// 深度/模板状态：写入模板值
	static ComPtr<ID3D11DepthStencilState> DSSDrawWithStencil;	// 深度/模板状态：对指定模板值的区域进行绘制
	static ComPtr<ID3D11DepthStencilState> DSSNoDoubleBlend;	// 深度/模板状态：无二次混合区域
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthTest;		// 深度/模板状态：关闭深度测试
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthWrite;		// 深度/模板状态：仅深度测试，不写入深度值

};


