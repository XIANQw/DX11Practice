#include "TestVLM.hlsli"

VertexPosHWNormalTex VS_3D(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);

    if (g_UseSH) {
        // Compute indirection UVs from world position
        float3 IndirectionVolumeUVs = clamp(posW * VLMWorldToUVScale + VLMWorldToUVAdd, 0.0f, .99f);
        float3 IndirectionTextureTexelCoordinate = IndirectionVolumeUVs * VLMIndirectionTextureSize;
        float4 BrickOffsetAndSize = g_IndirectionTexture.Load(int4(IndirectionTextureTexelCoordinate, 0.0f));

        float PaddedBrickSize = VLMBrickSize + 1;
        float3 BrickUV = (BrickOffsetAndSize.xyz * PaddedBrickSize + frac(IndirectionTextureTexelCoordinate / BrickOffsetAndSize.w) * VLMBrickSize + .5f) * VLMBrickTexelSize;

        SHCoefs2BandRGB IrradianceSH = GetVolumetricLightmapSH2(BrickUV);
        vOut.VertexIndirectSH[0] = IrradianceSH.R.coefs1_4;
        vOut.VertexIndirectSH[1] = IrradianceSH.G.coefs1_4;
        vOut.VertexIndirectSH[2] = IrradianceSH.B.coefs1_4;
    }


    //若当前在绘制阴影，先进行投影操作
    [flatten]
    if (g_IsShadow)
    {
        posW = mul(posW, g_Shadow);
    }

    vOut.PosH = mul(posW, viewProj);
    vOut.PosW = posW.xyz;
    vOut.NormalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
    vOut.Tex = vIn.Tex;
    return vOut;
}