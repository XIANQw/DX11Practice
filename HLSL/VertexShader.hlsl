#include "Shape.hlsli"

VS_OUT VS(VS_IN input) {
	VS_OUT res;
	matrix viewProj = mul(g_View, g_Proj);
	float4 posW = mul(float4(input.pos, 1.0f), g_World);
	
	res.posH = mul(posW, viewProj);
	res.posW = posW.xyz;
	res.normalW = mul(input.normal, (float3x3)g_WorldInvTranspose);
	res.color = input.color;
	return res;
}