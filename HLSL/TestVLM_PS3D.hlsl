#include "TestVLM.hlsli"



// 像素着色器(3D)
float4 PS_3D(VertexPosHWNormalTex pIn) : SV_Target
{
	SHCoefs3BandRGB IrradianceSH = (SHCoefs3BandRGB)0;
	if (g_UseSH) {
		if (g_SHMode == 0) {
			IrradianceSH.R.coefs1_4 = pIn.VertexIndirectSH[0];
			IrradianceSH.G.coefs1_4 = pIn.VertexIndirectSH[1];
			IrradianceSH.B.coefs1_4 = pIn.VertexIndirectSH[2];
		}
		else if (g_SHMode == 1) {
			float3 BrickUV = ComputeVolumetricLightmapBrickTextureUVs(pIn.PosW);

			SHCoefs2BandRGB IrradianceSH2 = GetVolumetricLightmapSH2(BrickUV);
			IrradianceSH.R.coefs1_4 = IrradianceSH2.R.coefs1_4;
			IrradianceSH.G.coefs1_4 = IrradianceSH2.G.coefs1_4;
			IrradianceSH.B.coefs1_4 = IrradianceSH2.B.coefs1_4;
		}
		else {
			float3 BrickUV = ComputeVolumetricLightmapBrickTextureUVs(pIn.PosW);
			IrradianceSH = GetVolumetricLightmapSH3(BrickUV);
		}
	}
	float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_TextureUsed) {
		// 提前进行裁剪，对不符合要求的像素可以避免后续运算
		texColor = g_Tex2D.Sample(g_Sam, pIn.Tex);
		clip(texColor.a - 0.1f);
	}

	// 标准化法向量
	pIn.NormalW = normalize(pIn.NormalW);

	// 顶点指向眼睛的向量
	float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

	// 初始化为0 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 indirectDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 directDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
	int i;
	
	bool useLight = g_UseDirLight || g_UsePointLight;

	if (g_UseSH) {
		SHCoefs3Band DiffuseTransferSH = CalcDiffuseTransferSH3(pIn.NormalW, 1);
        indirectDiffuse = max(float4(0, 0, 0, 0), float4(DotSH3(IrradianceSH, DiffuseTransferSH), 0.0f)) / 3.1415926f;
    }

	if (useLight) {
		if (g_UseDirLight) {
			[unroll]
			for (i = 0; i < DirLightNums; ++i)
			{
				ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.NormalW, toEyeW, A, D, S);
				ambient += A;
                directDiffuse += D;
				spec += S;
			}
		}
		// 若当前在绘制反射物体，需要对光照进行反射矩阵变换
		if (g_UsePointLight) {
			[unroll]
			for (i = 0; i < PointLightNums; ++i)
			{
				ComputePointLight(g_Material, g_PointLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
				ambient += A;
                directDiffuse += D;
				spec += S;
			}
		}
		 //若当前在绘制反射物体，需要对光照进行反射矩阵变换
		[unroll]
		for (i = 0; i < SpotLightNums; ++i)
		{
			ComputeSpotLight(g_Material, g_SpotLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
			ambient += A;
            directDiffuse += D;
			spec += S;
		}
	}
	 // diffuse = indirectDiffuse + directDiffuse(1 - indirectDiffuse)
     diffuse = lerp(indirectDiffuse, float4(1.0f, 1.0f, 1.0f, 1.0f), directDiffuse);
	
	float4 litColor = texColor * (ambient + diffuse) + spec;
	litColor.a = texColor.a * g_Material.Diffuse.a;
	return litColor;
}
