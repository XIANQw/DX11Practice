#include "Ex13_CBuffers.hlsli"

/*********************************
    ������ɫ��(2D)
    ���㶥����2DͶӰ�ռ��λ��,
    ��Ϊ��������2D�������Բ���Ҫ���Ա任
**********************************/
VertexPosHTex VS_2D(VertexPosTex vIn)
{
    VertexPosHTex vOut;
    vOut.PosH = float4(vIn.PosL, 1.0f);
    vOut.Tex = vIn.Tex;
    return vOut;
}