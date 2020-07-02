#include "Shape.hlsli"

float4 PS(VS_OUT input) : SV_Target
{
	return input.color;
}