#include "TestVLM.hlsli"

float3 ComputeVolumetricLightmapBrickTextureUVs(float3 WorldPosition)
{
	// Compute indirection UVs from world position
	float3 IndirectionVolumeUVs = clamp(WorldPosition * VLMWorldToUVScale + VLMWorldToUVAdd, 0.0f, .99f);
	float3 IndirectionTextureTexelCoordinate = IndirectionVolumeUVs * VLMIndirectionTextureSize;
	float4 BrickOffsetAndSize = g_IndirectionTexture.Load(int4(IndirectionTextureTexelCoordinate, 0.0f));

	float PaddedBrickSize = VLMBrickSize + 1;
	return (BrickOffsetAndSize.xyz * PaddedBrickSize + frac(IndirectionTextureTexelCoordinate / BrickOffsetAndSize.w) * VLMBrickSize + .5f) * VLMBrickTexelSize;
}

// ������ɫ��(3D)
float4 PS_3D(VertexPosHWNormalTex pIn) : SV_Target
{
	SHCoefs3BandRGB IrradianceSH = (SHCoefs3BandRGB)0;
	if (g_UseSH) {
		IrradianceSH.R.coefs1_4 = pIn.VertexIndirectSH[0];
		IrradianceSH.G.coefs1_4 = pIn.VertexIndirectSH[1];
		IrradianceSH.B.coefs1_4 = pIn.VertexIndirectSH[2];
	}
	float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_TextureUsed) {
		// ��ǰ���вü����Բ�����Ҫ������ؿ��Ա����������
		texColor = g_Tex2D.Sample(g_Sam, pIn.Tex);
		clip(texColor.a - 0.1f);
	}

	// ��׼��������
	pIn.NormalW = normalize(pIn.NormalW);

	// ����ָ���۾�������
	float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

	// ��ʼ��Ϊ0 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
	int i;

	[unroll]
	for (i = 0; i < 5; ++i)
	{
		if (g_UseSH) ComputeDirectionalLightSH(g_Material, g_DirLight[i], IrradianceSH, pIn.NormalW, toEyeW, A, D, S);
		else ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}


	// ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
	[unroll]
	for (i = 0; i < 5; ++i)
	{
		if (g_UseSH) ComputePointLightSH(g_Material, g_PointLight[i], IrradianceSH, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		else ComputePointLight(g_Material, g_PointLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}


	// ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
	[unroll]
	for (i = 0; i < 5; ++i)
	{
		if (g_UseSH) ComputeSpotLightSH(g_Material, g_SpotLight[i], IrradianceSH, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		else ComputeSpotLight(g_Material, g_SpotLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}


	float4 litColor = texColor * (ambient + diffuse) + spec;
	litColor.a = texColor.a * g_Material.Diffuse.a;
	return litColor;
}
