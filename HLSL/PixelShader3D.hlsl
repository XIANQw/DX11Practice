#include "Shape.hlsli"

float4 PS_3D(VertexPosHNormalTex pIn) : SV_TARGET
{
	//标准化法向量
	pIn.normalW = normalize(pIn.normalW);

	//顶点指向眼睛的向量
	float3 toEye = normalize(g_EyePosW - pIn.posW);

	// 初始化
	float4 ambient, diffuse, specular, A, D, S;
	ambient = diffuse = specular = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 平行光计算
	for (int i = 0; i < g_NumDirLight; i++) {
		ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}

	// 点光计算
	for (int i = 0; i < g_NumPointLight; i++) {
		ComputePointLight(g_Material, g_PointLight[i], pIn.posW, pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}

	// 聚光计算
	for (int i = 0; i < g_NumSpotLight; i++) {
		ComputeSpotLight(g_Material, g_SpotLight[i], pIn.posW, pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}
	 
	//Texture2D类 提供Sample方法，需要提供采样器状态和2D纹理坐标方可使用，然后返回一个包含RGBA信息的float4向量
	float texColor = g_Tex.Sample(g_SamLinear, pIn.Tex);
	float4 litColor = texColor * (ambient + diffuse) + specular;
	litColor.a = texColor.a * g_Material.Diffuse.a;
	return litColor;
}