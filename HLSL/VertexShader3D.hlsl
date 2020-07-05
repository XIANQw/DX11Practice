#include "Shape.hlsli"

VertexPosHNormalTex VS_3D(VertexPosNormalTex vIn)
{
	VertexPosHNormalTex pOut;
	matrix viewProj = mul(g_View, g_Proj);
	float4 posW = mul(float(vIn.PosL, 1.0f), g_World);

	vOut.PosH = mul(posW, viewProj);
	vOut.PosW = posW.xyz;
	vOut.NormalW = mul(vIn.NormalL, (float3x3)g_WorldInvTranspose);
	vOut.Tex = vIn.Tex;
	return vOut;
}