#include "CBuffer.hlsli"

/**********************************
    像素着色器(2D), 用来渲染火焰bmp
***********************************/
float4 PS_2D(VertexPosHTex pIn) : SV_Target
{
    return g_Tex.Sample(g_SamLinear, pIn.Tex);
}