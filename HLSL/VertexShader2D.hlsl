#include "Shape.hlsli"

VertexPosHTex VS_2D(VertexPosTex vIn )
{
	VertexPosHTex vOut;
	vOut.PosL = float4(vIn.PosL, 1.0f);
	vOut.Tex = vIn.Tex;
	return vOut;
}