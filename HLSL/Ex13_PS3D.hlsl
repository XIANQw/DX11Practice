#include "Ex13_CBuffers.hlsli"

// ������ɫ��(3D)
float4 PS_3D(VertexPosHWNormalTex pIn) : SV_Target
{
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (g_TextureUsed) {
        // ��ǰ���вü����Բ�����Ҫ������ؿ��Ա����������
        texColor = g_Tex.Sample(g_Sam, pIn.Tex);
        clip(texColor.a - 0.1f);
    }

    // ��׼��������
    pIn.NormalW = normalize(pIn.NormalW);

    // ����ָ���۾�������
    float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

    // ��ʼ��Ϊ0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;

    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }




    // ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputePointLight(g_Material, g_PointLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }


    // ����ǰ�ڻ��Ʒ������壬��Ҫ�Թ��ս��з������任
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeSpotLight(g_Material, g_SpotLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }


    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.Diffuse.a;
    return litColor;
}