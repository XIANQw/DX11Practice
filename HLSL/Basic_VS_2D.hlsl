#include "Basic.hlsli"

/*********************************
    顶点着色器(2D)
    计算顶点在2D投影空间的位置,
    因为本来就是2D坐标所以不需要线性变换
**********************************/ 
VertexPosHTex VS_2D(VertexPosTex vIn)
{
    VertexPosHTex vOut;
    vOut.PosH = float4(vIn.PosL, 1.0f);
    vOut.Tex = vIn.Tex;
    return vOut;
}