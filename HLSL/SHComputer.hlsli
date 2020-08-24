Texture3D<uint4> g_IndirectionTexture: register(t1);

Texture3D g_AmbientVector: register(t2);
Texture3D g_SHCoef0: register(t3);
Texture3D g_SHCoef1: register(t4);
Texture3D g_SHCoef2: register(t5);
Texture3D g_SHCoef3: register(t6);
Texture3D g_SHCoef4: register(t7);
Texture3D g_SHCoef5: register(t8);
Texture3D g_SHCoef0Phase1 : register(t9);
Texture3D g_SHCoef0Phase2 : register(t10);
Texture3D g_SHCoef0Phase3 : register(t11);
Texture3D g_SHCoef0Phase4 : register(t12);
Texture3D g_SHCoef0Phase5 : register(t13);

SamplerState g_Sam : register(s0);

struct SHCoefs1Band {
	half coef1;
};

struct SHCoefs1BandRGB {
	SHCoefs1Band R;
	SHCoefs1Band G;
	SHCoefs1Band B;
};

struct SHCoefs2Band {
	half4 coefs1_4;
};

struct SHCoefs2BandRGB {
	SHCoefs2Band R;
	SHCoefs2Band G;
	SHCoefs2Band B;
};

struct SHCoefs3Band {
	half4 coefs1_4;
	half4 coefs5_8;
	half coef9;
};

struct SHCoefs3BandRGB {
	SHCoefs3Band R;
	SHCoefs3Band G;
	SHCoefs3Band B;
};

half DotSH3(SHCoefs3Band A, SHCoefs3Band B)
{
	half Result = dot(A.coefs1_4, B.coefs1_4);
	Result += dot(A.coefs5_8, B.coefs5_8);
	Result += A.coef9 * B.coef9;
	return Result;
}

half3 DotSH3(SHCoefs3BandRGB A, SHCoefs3Band B)
{
	half3 Result = 0;
	Result.r = DotSH3(A.R, B);
	Result.g = DotSH3(A.G, B);
	Result.b = DotSH3(A.B, B);
	return Result;
}

float4  Texture3DSampleLevel(Texture3D Tex, SamplerState Sampler, float3 UV, float  Mip)
{
	return Tex.SampleLevel(Sampler, UV, Mip);
}


SHCoefs3Band SHBasisFunction3(half3 InputVector)
{
	SHCoefs3Band Result;
	// These are derived from simplifying SHBasisFunction in C++

	// band0
	Result.coefs1_4.x = 0.282095f;

	// band1
	Result.coefs1_4.y = -0.488603f * InputVector.y;
	Result.coefs1_4.z = 0.488603f * InputVector.z;
	Result.coefs1_4.w = -0.488603f * InputVector.x;

	// band2
	half3 VectorSquared = InputVector * InputVector;
	Result.coefs5_8.x = 1.092548f * InputVector.x * InputVector.y;
	Result.coefs5_8.y = -1.092548f * InputVector.y * InputVector.z;
	Result.coefs5_8.z = 0.315392f * (3.0f * VectorSquared.z - 1.0f);
	Result.coefs5_8.w = -1.092548f * InputVector.x * InputVector.z;
	Result.coef9 = 0.546274f * (VectorSquared.x - VectorSquared.y);

	return Result;
}

SHCoefs3Band CalcDiffuseTransferSH3(half3 Normal, half Exponent)
{
	half PI = 3.1415926f;
	SHCoefs3Band Result = SHBasisFunction3(Normal);

	// These formula are scaling factors for each SH band that convolve a SH with the circularly symmetric function
	// max(0,cos(theta))^Exponent
	half L0 = 2 * PI / (1 + 1 * Exponent);
	half L1 = 2 * PI / (2 + 1 * Exponent);
	half L2 = Exponent * 2 * PI / (3 + 4 * Exponent + Exponent * Exponent);
	half L3 = (Exponent - 1) * 2 * PI / (8 + 6 * Exponent + Exponent * Exponent);

	// Multiply the coefficients in each band with the appropriate band scaling factor.
	Result.coefs1_4.x *= L0;
	Result.coefs1_4.yzw *= L1;
	Result.coefs5_8.xyzw *= L2;
	Result.coef9 *= L2;

	return Result;
}

void GetVolumetricLightmapSHCoefficients0(float3 BrickTextureUVs, out float3 AmbientVector, out float4 SHCoefs0R, out float4 SHCoefs0G, out float4 SHCoefs0B)
{
	AmbientVector = Texture3DSampleLevel(g_AmbientVector, g_Sam, BrickTextureUVs, 0).xyz;
	// get SHc0 rgb from SHc[0][2][4]
	SHCoefs0R = Texture3DSampleLevel(g_SHCoef0, g_Sam, BrickTextureUVs, 0) * 2 - 1;
	SHCoefs0G = Texture3DSampleLevel(g_SHCoef2, g_Sam, BrickTextureUVs, 0) * 2 - 1;
	SHCoefs0B = Texture3DSampleLevel(g_SHCoef4, g_Sam, BrickTextureUVs, 0) * 2 - 1;

	// Undo normalization done in FIrradianceBrickData::SetFromVolumeLightingSample
	float4 SHDenormalizationScales0 = float4(
		0.488603f / 0.282095f,
		0.488603f / 0.282095f,
		0.488603f / 0.282095f,
		1.092548f / 0.282095f);

	SHCoefs0R = SHCoefs0R * AmbientVector.x * SHDenormalizationScales0;
	SHCoefs0G = SHCoefs0G * AmbientVector.y * SHDenormalizationScales0;
	SHCoefs0B = SHCoefs0B * AmbientVector.z * SHDenormalizationScales0;
}


SHCoefs2BandRGB GetVolumetricLightmapSH2(float3 BrickTextureUVs)
{
	float3 AmbientVector;
	float4 SHCoefs0R;
	float4 SHCoefs0G;
	float4 SHCoefs0B;
	GetVolumetricLightmapSHCoefficients0(BrickTextureUVs, AmbientVector, SHCoefs0R, SHCoefs0G, SHCoefs0B);

	SHCoefs2BandRGB IrradianceSH;

	// band1 3 coefs
	IrradianceSH.R.coefs1_4 = float4(AmbientVector.x, SHCoefs0R.xyz);
	IrradianceSH.G.coefs1_4 = float4(AmbientVector.y, SHCoefs0G.xyz);
	IrradianceSH.B.coefs1_4 = float4(AmbientVector.z, SHCoefs0B.xyz);

	return IrradianceSH;
}


SHCoefs3BandRGB GetVolumetricLightmapSH3(float3 BrickTextureUVs)
{
	float3 AmbientVector;
	float4 SHCoefs0R;
	float4 SHCoefs0G;
	float4 SHCoefs0B;
	GetVolumetricLightmapSHCoefficients0(BrickTextureUVs, AmbientVector, SHCoefs0R, SHCoefs0G, SHCoefs0B);

	float4 SHCoefs1R = Texture3DSampleLevel(g_SHCoef1, g_Sam, BrickTextureUVs, 0) * 2 - 1;
	float4 SHCoefs1G = Texture3DSampleLevel(g_SHCoef3, g_Sam, BrickTextureUVs, 0) * 2 - 1;
	float4 SHCoefs1B = Texture3DSampleLevel(g_SHCoef5, g_Sam, BrickTextureUVs, 0) * 2 - 1;

	float4 SHDenormalizationScales1 = float4(
		1.092548f / 0.282095f,
		4.0f * 0.315392f / 0.282095f,
		1.092548f / 0.282095f,
		2.0f * 0.546274f / 0.282095f);

	SHCoefs1R = SHCoefs1R * AmbientVector.x * SHDenormalizationScales1;
	SHCoefs1G = SHCoefs1G * AmbientVector.y * SHDenormalizationScales1;
	SHCoefs1B = SHCoefs1B * AmbientVector.z * SHDenormalizationScales1;

	SHCoefs3BandRGB IrradianceSH;
	// Construct the SH environment
	IrradianceSH.R.coefs1_4 = float4(AmbientVector.x, SHCoefs0R.xyz);
	IrradianceSH.R.coefs5_8 = float4(SHCoefs0R.w, SHCoefs1R.xyz);
	IrradianceSH.R.coef9 = SHCoefs1R.w;

	IrradianceSH.G.coefs1_4 = float4(AmbientVector.y, SHCoefs0G.xyz);
	IrradianceSH.G.coefs5_8 = float4(SHCoefs0G.w, SHCoefs1G.xyz);
	IrradianceSH.G.coef9 = SHCoefs1G.w;

	IrradianceSH.B.coefs1_4 = float4(AmbientVector.z, SHCoefs0B.xyz);
	IrradianceSH.B.coefs5_8 = float4(SHCoefs0B.w, SHCoefs1B.xyz);
	IrradianceSH.B.coef9 = SHCoefs1B.w;

	return IrradianceSH;
}

