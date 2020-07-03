#include "Shape.hlsli"

float4 PS(VS_OUT input) : SV_Target
{
	//��׼��������
	input.normalW = normalize(input.normalW);
	
	//����ָ���۾�������
	float3 toEye = normalize(g_EyePosW - input.posW);
	
	// ��ʼ��
	float4 ambient, diffuse, specular, A, D, S;
	ambient = diffuse = specular = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ƽ�й����
	ComputeDirectionalLight(g_Material, g_DirLight, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;
	
	// ������
	ComputePointLight(g_Material, g_PointLight, input.posW, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	// �۹����
	ComputeSpotLight(g_Material, g_SpotLight, input.posW, input.normalW, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	float4 litColor = input.color * (ambient + diffuse) + specular;
	litColor.a = g_Material.Diffuse.a * input.color.a;
	return litColor;
}

