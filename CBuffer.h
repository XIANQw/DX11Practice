#pragma once
#include <DirectXMath.h>
#include "light.h"

/*******************************************************************************************
*                                    CBuffer.h                                             *
*由于参数不断增多，每个常量缓冲区都在增大，如果只为了修改一个变量而重新读取整个缓冲区感觉很亏，      *
*所以现在把这些变量按照刷新频率分成四个等级，每次绘制更新，每帧更新，每次改变窗口大小更新和不经常更新 *
********************************************************************************************/


struct CBChangesEveryDrawing {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX worldInvTranspose;
};

struct CBChangesEveryFrame {
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX eysPos;
};

struct CBChangesOnResize {
	DirectX::XMMATRIX proj;
};

struct CBChangesRarely {
    DirectionalLight dirLight[10];
    PointLight pointLight[10];
    SpotLight spotLight[10];
    Material material;
    int numDirLight;
    int numPointLight;
    int numSpotLight;
    float pad;
};
