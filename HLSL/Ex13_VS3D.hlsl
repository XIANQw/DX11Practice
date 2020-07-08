#include "Ex13_CBuffers.hlsli"

/*************************************************
 *   ������ɫ��(3D)                               *
    1 ����ģ�Ϳռ��µ�3D������2DͶӰ�ռ������      *
 *  �����ö����posL������world���Ա任, �õ�       *
 *  2 ����ռ����� posW, Ȼ���ٽ���view �� proj    *
 *  �任��ͶӰ�ռ��2D����.                        *
 *  3 ����ռ䷨���� normalW �ɶ����ģ�Ϳռ䷨����  *
 *  ��������ת������� g_WorldInvTranspose �õ�.    *
 *                                     -XIANQw    *
 *************************************************/
VertexPosHWNormalTex VS_3D(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);

    // ����ǰ�ڻ�����Ӱ���Ƚ���ͶӰ����
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