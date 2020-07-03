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
	if (diffuseFactor > 0.0f) {
		float v = reflect(-lightVector, normal);
		float specularFactor = pow(max(dot(v, view), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		specular = specularFactor * mat.Specular * light.Specular;
	}
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
	Material g_Material;
	DirectionalLight g_DirLight;
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
