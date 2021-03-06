
// 方向光
struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

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

// 聚光灯
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

// 物体表面材质
struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecPower
    float4 Reflect;
};


void ComputeDirectionalLight(Material mat, DirectionalLight L,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
    half PI = 3.1415926f;
	// 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);


	// 光向量与照射方向相反
    float3 lightVec = -L.Direction;

	// 添加环境光
    ambient = mat.Ambient * L.Ambient;

	// 添加漫反射光
    float diffuseFactor = max(dot(lightVec, normal), 0.0f);
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;

	    // 镜面光
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        spec = specFactor * mat.Specular * L.Specular;
    }
}


void ComputePointLight(Material mat, PointLight L,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
    half PI = 3.1415926f;
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
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        spec = specFactor * mat.Specular * L.Specular;
    }


	// 光的衰弱
    float att = (L.Range - d) / L.Range;

    diffuse *= att;
    spec *= att;
}


void ComputeSpotLight(Material mat, SpotLight L,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
    half PI = 3.1415926f;
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
    if (diffuseFactor > 0.0f)
    {
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        spec = specFactor * mat.Specular * L.Specular;
    }
	// 计算汇聚因子和衰弱系数
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    float att = spot / dot(L.Att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}
