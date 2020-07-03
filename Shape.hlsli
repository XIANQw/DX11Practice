struct Material {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
};

// 平行光模型
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
	// 光向量是光源照射方向的反方向
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

// 点光
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
	// 初始化输出
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 从表面到光源的向量
	float3 lightVec = L.Position - pos;

	// 表面到光线的距离
	float d = length(lightVec);

	// 灯光范围测试
	if (d > L.Range)
		return;

	// 标准化光向量
	lightVec /= d;

	// 环境光计算
	ambient = mat.Ambient * L.Ambient;

	// 漫反射和镜面计算
	float diffuseFactor = dot(lightVec, normal);

	// 展开以避免动态分支
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// 光的衰弱
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
	// 初始化输出
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// // 从表面到光源的向量
	float3 lightVec = L.Position - pos;

	// 表面到光源的距离
	float d = length(lightVec);

	// 范围测试
	if (d > L.Range)
		return;

	// 标准化光向量
	lightVec /= d;

	// 计算环境光部分
	ambient = mat.Ambient * L.Ambient;


	// 计算漫反射光和镜面反射光部分
	float diffuseFactor = dot(lightVec, normal);

	// 展开以避免动态分支
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// 计算汇聚因子和衰弱系数
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	spec *= att;
}



// VS常量缓冲存于 寄存器b0
cbuffer VSConstantBuffer: register(b0) {
	matrix g_World;
	matrix g_View;
	matrix g_Proj;
	matrix g_WorldInvTranspose;
}

// PS常量缓冲存于 寄存器b1
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
	float3 posW: POSITION; // 世界坐标
	float3 normalW: NORMAL; //法向量在世界空间的方向 
	float4 color: COLOR;
};
