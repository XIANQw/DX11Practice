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
    int g_SHMode;

    int g_UseBrickId;
    float3 pad;
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
    DirectionalLight g_DirLight[20];
    PointLight g_PointLight[20];
    SpotLight g_SpotLight[20];
    int DirLightNums;
    int PointLightNums;
    int SpotLightNums;
    int Rarely_pad1;
}

cbuffer CBVLMParams : register(b5)
{
    float3 VLMWorldToUVScale;
    float VLMBrickSize;
    float3 VLMIndirectionTextureSize;
    float VLMParams_pad1;
    float3 VLMWorldToUVAdd;
    float VLMParams_pad2;
    float3 VLMBrickTexelSize;
    bool VLMParams_pad3;
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


int3 GetCurrentBrick() {
    int3 res;
    int3 layout = int3(1.0f / VLMBrickTexelSize.x, 1.0f / VLMBrickTexelSize.y, 1.0f / VLMBrickTexelSize.z);
    res.x = g_UseBrickId % layout.x;
    res.y = g_UseBrickId / layout.x % layout.y;
    res.z = g_UseBrickId / (layout.x * layout.y);
    return res;
}

float3 ComputeVolumetricLightmapBrickTextureUVs(float3 WorldPosition)
{
    // Compute indirection UVs from world position
    float3 IndirectionVolumeUVs = clamp(WorldPosition * VLMWorldToUVScale + VLMWorldToUVAdd, 0.0f, .99f);
    float3 IndirectionTextureTexelCoordinate = IndirectionVolumeUVs * VLMIndirectionTextureSize;

    float4 BrickOffsetAndSize = g_IndirectionTexture.Load(int4(IndirectionTextureTexelCoordinate, 0.0f));

    float PaddedBrickSize = VLMBrickSize + 1;
    return (BrickOffsetAndSize.xyz * PaddedBrickSize + frac(IndirectionTextureTexelCoordinate / BrickOffsetAndSize.w) * VLMBrickSize + .5f) * VLMBrickTexelSize;
}
