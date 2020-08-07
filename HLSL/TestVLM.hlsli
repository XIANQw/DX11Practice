#include "SHLight.hlsli"

Texture2D g_Tex2D: register(t0);


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}

cbuffer CBDrawingState:register(b1) {
    int g_IsReflection;
    int g_IsShadow;
    int g_TextureUsed;
    int g_UseSH;

    int g_UseLight;
    int g_UseDirLight;
    int g_UsePointLight;
    float pad;
}

cbuffer CBChangesEveryFrame : register(b2)
{
    matrix g_View;
    float3 g_EyePosW;
    float g_Pad2;
}

cbuffer CBChangesOnResize : register(b3)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b4)
{
    matrix g_Reflection;
    matrix g_RefShadow;
    matrix g_Shadow;
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

cbuffer CBVLMParams : register(b5)
{
    float3 VLMWorldToUVScale;
    float VLMBrickSize;
    float3 VLMIndirectionTextureSize;
    float pad1;
    float3 VLMWorldToUVAdd;
    float pad2;
    float3 VLMBrickTexelSize;
    bool pad3;
};

struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 NormalW : NORMAL; // 法向量在世界中的方向
    float2 Tex : TEXCOORD;
    float4 VertexIndirectSH[3] : TEXCOORD14;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

