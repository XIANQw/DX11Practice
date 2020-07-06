#pragma once
#include <DirectXMath.h>
#include "light.h"

/*******************************************************************************************
*                                    CBuffer.h                                             *
*���ڲ����������࣬ÿ�����������������������ֻΪ���޸�һ�����������¶�ȡ�����������о��ܿ���      *
*�������ڰ���Щ��������ˢ��Ƶ�ʷֳ��ĸ��ȼ���ÿ�λ��Ƹ��£�ÿ֡���£�ÿ�θı䴰�ڴ�С���ºͲ��������� *
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
