#include "Shape.hlsli"

float4 PS_3D(VertexPosHNormalTex pIn) : SV_TARGET
{
	//��׼��������
	pIn.normalW = normalize(pIn.normalW);

	//����ָ���۾�������
	float3 toEye = normalize(g_EyePosW - pIn.posW);

	// ��ʼ��
	float4 ambient, diffuse, specular, A, D, S;
	ambient = diffuse = specular = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ƽ�й����
	for (int i = 0; i < g_NumDirLight; i++) {
		ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}

	// ������
	for (int i = 0; i < g_NumPointLight; i++) {
		ComputePointLight(g_Material, g_PointLight[i], pIn.posW, pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}

	// �۹����
	for (int i = 0; i < g_NumSpotLight; i++) {
		ComputeSpotLight(g_Material, g_SpotLight[i], pIn.posW, pIn.normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}
	 
	//Texture2D�� �ṩSample��������Ҫ�ṩ������״̬��2D�������귽��ʹ�ã�Ȼ�󷵻�һ������RGBA��Ϣ��float4����
	float texColor = g_Tex.Sample(g_SamLinear, pIn.Tex);
	float4 litColor = texColor * (ambient + diffuse) + specular;
	litColor.a = texColor.a * g_Material.Diffuse.a;
	return litColor;
}