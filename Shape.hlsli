struct Material {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
};

// ƽ�й�ģ��
struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float Pad;
};

void ComputeDirectionalLight(Material mat, DirectionalLight light, 
	float3 normal, float3 view, 
	out float4 ambient, out float4 diffuse, out float4 specular) {
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// �������ǹ�Դ���䷽��ķ�����
	float3 lightVector = -light.Direction;
	ambient = mat.Ambient * light.Ambient;
	float diffuseFactor = dot(lightVector, normal);
	[flatten]
	if (diffuseFactor > 0.0f) {
		float v = reflect(-lightVector, normal);
		float specularFactor = pow(max(dot(v, view), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		specular = specularFactor * mat.Specular * light.Specular;
	}
}

// ���
struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float Pad;
};

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
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
	float diffuseFactor = dot(lightVec, normal);

	// չ���Ա��⶯̬��֧
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// ���˥��
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	diffuse *= att;
	spec *= att;
}

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float Pad;
};


void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
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
	float diffuseFactor = dot(lightVec, normal);

	// չ���Ա��⶯̬��֧
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// ���������Ӻ�˥��ϵ��
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	spec *= att;
}



// VS����������� �Ĵ���b0
cbuffer VSConstantBuffer: register(b0) {
	matrix g_World;
	matrix g_View;
	matrix g_Proj;
	matrix g_WorldInvTranspose;
}

// PS����������� �Ĵ���b1
cbuffer PSConstantBuffer: register(b1) {
	DirectionalLight g_DirLight;
	PointLight g_PointLight;
	SpotLight g_SpotLight;
	Material g_Material;
	float3 g_EyePosW;
	float g_Pad;
}


struct VS_IN {
	float3 pos: POSITION;
	float3 normal : NORMAL;
	float4 color: COLOR;
};

struct VS_OUT {
	float4 posH:SV_POSITION;
	float3 posW: POSITION; // ��������
	float3 normalW: NORMAL; //������������ռ�ķ��� 
	float4 color: COLOR;
};
