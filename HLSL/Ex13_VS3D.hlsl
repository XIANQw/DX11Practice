#include "Ex13_CBuffers.hlsli"

/*************************************************
 *   顶点着色器(3D)                               *
    1 计算模型空间下的3D顶点在2D投影空间的坐标      *
 *  所以用顶点的posL进行了world线性变换, 得到       *
 *  2 世界空间坐标 posW, 然后再进行view 和 proj    *
 *  变换成投影空间的2D坐标.                        *
 *  3 世界空间法向量 normalW 由顶点的模型空间法向量  *
 *  乘上世界转置逆矩阵 g_WorldInvTranspose 得到.    *
 *                                     -XIANQw    *
 *************************************************/
VertexPosHWNormalTex VS_3D(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);

    // 若当前在绘制阴影，先进行投影操作
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