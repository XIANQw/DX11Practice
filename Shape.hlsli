cbuffer ConstantBuffer: register(b0) {
	matrix g_World;
	matrix g_View;
	matrix g_Proj;
}


struct VS_IN {
	float3 pos: POSITION;
	float4 color: COLOR;
};

struct VS_OUT {
	float4 posH:SV_POSITION;
	float4 color: COLOR;
};
