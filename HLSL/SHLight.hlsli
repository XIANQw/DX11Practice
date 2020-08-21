#include "LightHelper.hlsli"
#include "SHComputer.hlsli"

void ComputeDirectionalLightSH(Material mat, DirectionalLight L,
	SHCoefs3BandRGB IrradianceSH,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	half PI = 3.1415926f;
	// ��ʼ�����
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);


	// �����������䷽���෴
	float3 lightVec = -L.Direction;

	// ��ӻ�����
	ambient = mat.Ambient * L.Ambient;

	// ����������;����
	float diffuseFactor = max(dot(lightVec, normal), 0.0f);

	SHCoefs3Band DiffuseTransferSH = CalcDiffuseTransferSH3(normal, 1);
	diffuse = max(float4(0, 0, 0, 0), float4(DotSH3(IrradianceSH, DiffuseTransferSH), 0.0f)) / PI;
	diffuse *= diffuseFactor * mat.Diffuse * L.Diffuse;

	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	spec = specFactor * mat.Specular * L.Specular;

}


void ComputePointLightSH(Material mat, PointLight L,
	SHCoefs3BandRGB IrradianceSH,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	half PI = 3.1415926f;
	// ��ʼ�����
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// �ӱ��浽��Դ������
	float3 lightVec = L.Position - pos;

	// ���浽���ߵľ���
	float d = length(lightVec);

	// �ƹⷶΧ����
	if (d > L.Range)
		return;

	// ��׼��������
	lightVec /= d;

	// ���������
	ambient = mat.Ambient * L.Ambient;

	// ������;������
	float diffuseFactor = max(dot(lightVec, normal), 0.0f);

	SHCoefs3Band DiffuseTransferSH = CalcDiffuseTransferSH3(normal, 1);
	diffuse = max(float4(0, 0, 0, 0), float4(DotSH3(IrradianceSH, DiffuseTransferSH), 0.0f)) / PI;
	diffuse *= diffuseFactor * mat.Diffuse * L.Diffuse;

	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	spec = specFactor * mat.Specular * L.Specular;

	// ���˥��
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	diffuse *= att;
	spec *= att;
}


void ComputeSpotLightSH(Material mat, SpotLight L,
	SHCoefs3BandRGB IrradianceSH,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	half PI = 3.1415926f;
	// ��ʼ�����
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// // �ӱ��浽��Դ������
	float3 lightVec = L.Position - pos;

	// ���浽��Դ�ľ���
	float d = length(lightVec);

	// ��Χ����
	if (d > L.Range)
		return;

	// ��׼��������
	lightVec /= d;

	// ���㻷���ⲿ��
	ambient = mat.Ambient * L.Ambient;


	// �����������;��淴��ⲿ��
	float diffuseFactor = max(dot(lightVec, normal), 0.0f);

	SHCoefs3Band DiffuseTransferSH = CalcDiffuseTransferSH3(normal, 1);
	diffuse = max(float4(0, 0, 0, 0), float4(DotSH3(IrradianceSH, DiffuseTransferSH), 0.0f)) / PI;
	diffuse *= diffuseFactor * mat.Diffuse * L.Diffuse;

	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	spec = specFactor * mat.Specular * L.Specular;


	// ���������Ӻ�˥��ϵ��
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	spec *= att;
}

