#include "Shape.hlsli"

float4 PS(VS_OUT input) : SV_Target
{
	//标准化法向量
	input.normalW = normalize(input.normalW);
	
	//顶点指向眼睛的向量
	float3 toEye = normalize(g_EyePosW - input.posW);
	
	// 初始化
	float4 ambient, diffuse, specular, A, D, S;
	ambient = diffuse = specular = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 平行光计算
	ComputeDirectionalLight(g_Material, g_DirLight, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;
	
	// 点光计算
	ComputePointLight(g_Material, g_PointLight, input.posW, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	// 聚光计算
	ComputeSpotLight(g_Material, g_SpotLight, input.posW, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	float4 litColor = input.color * (ambient + diffuse) + specular;
	litColor.a = g_Material.Diffuse.a * input.color.a;
	return litColor;
}

