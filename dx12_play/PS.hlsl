#define ALPHA_TEST = 1

Texture2D g_texture : register(t5);
TextureCube g_skyTexture : register(t6);
SamplerState g_sampler : register(s0);

cbuffer cbPass : register(b2)
{
    float4x4 gViewProj;
    float3 gViewPos;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 posL : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    uint meshletIndex : COLOR0;
};

float4 PSMain(PSInput pIn) : SV_TARGET
{
    float4 textureColor = g_texture.Sample(g_sampler, pIn.uv);

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1. We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(textureColor.a - 0.1f);
#endif

    float3 normal = normalize(pIn.normal);
    
    // Diffuse
    float3 dir = normalize(float3(0, 1.0f, 0.0f));
    float diff = max(dot(normal, dir), 0.0);
    float3 diffuse = diff * 0.8;
    diffuse *= float3(.9, .9, 1.0); // Light Color

    // Specular
    float3 I = normalize(pIn.posL - gViewPos);
    float3 R = reflect(I, normal);
    float3 specular = 0.05 * g_skyTexture.Sample(g_sampler, R).xyz;

    // Ambient
    float4 aLightColor = float4(1,1,1,1);
    float ambientStrength = 0.2;
    float3 ambient = ambientStrength * aLightColor;

    float3 result = (ambient + diffuse + specular) * textureColor;

    return float4(result, 1);
}