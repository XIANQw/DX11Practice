#include "RenderSponza.hlsli"

VertexPosHWNormalTangentTex VS_3D(VertexPosNormalTangentTex vIn) {
    VertexPosHWNormalTangentTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
    
    if (g_SHMode==0 && g_UseSH) {
        // Compute indirection UVs from world position
        float3 BrickUV = ComputeVolumetricLightmapBrickTextureUVs(posW);

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
    vOut.TangentW = mul(vIn.TangentL, g_World);
    vOut.Tex = vIn.Tex;
    return vOut;
}