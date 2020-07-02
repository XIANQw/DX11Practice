#include "Shape.hlsli"

VS_OUT VS(VS_IN input) {
	VS_OUT res;
	res.posH = mul(float4(input.pos, 1.0f), g_World);
	res.posH = mul(res.posH, g_View);
	res.posH = mul(res.posH, g_Proj);
	res.color = input.color;
	return res;
}