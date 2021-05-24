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
    float3 posW : POSITION0;
    float3 posS : POSITION1;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    uint meshletIndex : COLOR0;
};

float4 PSMain(PSInput pIn) : SV_TARGET
{
    float4 textureColor = g_texture.Sample(g_sampler, float2(pIn.uv.x*5,pIn.uv.y*2.5));

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
    float3 diffuse = diff * 0.96;
    diffuse *= float3(.9, .9, 1.0); // Light Color

    // Specular
    float3 I = normalize(pIn.posW - gViewPos);
    float3 R = reflect(I, normal);
    float3 specular = g_skyTexture.Sample(g_sampler, R).rgb;

    float Pi = 6.28318530718f; // Pi*2
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 3.0 - More is better but slower)
    float Size = 50; // BLUR SIZE (Radius)
    float2 Radius = Size / float2(900,900);

    // Blur calculations
    for (float d = 0.0; d < Pi; d += Pi / Directions)
    {
        for (float i = 1.0 / Quality; i <= 1.0; i += 1.0 / Quality)
        {
            specular += g_skyTexture.Sample(g_sampler, R + float3(cos(d), sin(d), 0) * float3(Radius, 0) * i).rgb;
        }
    }
    specular /= Quality * Directions - 15.0;
    specular *= 0.25;

    // Ambient
    float4 aLightColor = float4(1,1,1,1);
    float ambientStrength = 0.8;
    float3 ambient = ambientStrength * aLightColor;

    float3 result = (ambient + diffuse + specular) * textureColor;

    return float4(result, 1);
}