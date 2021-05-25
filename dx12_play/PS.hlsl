#define ALPHA_TEST = 1

Texture2D g_texture : register(t5);
TextureCube g_skyTexture : register(t6);
Texture2D g_normalTexture : register(t7);
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
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 tangentW : TANGENT;
    uint meshletIndex : COLOR0;
};

// --------------------------------------------------------------------
// Transforms a normal map sample to world space.
//--------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample,
    float3 unitNormalW,
    float3 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;
    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);
    return bumpedNormalW;
}

float4 PSMain(PSInput pIn) : SV_TARGET
{
    float4 textureColor = g_texture.Sample(g_sampler, float2(pIn.uv.x * 3, pIn.uv.y * 1.5));

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1. We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(textureColor.a - 0.1f);
#endif

    float4 normalMapSample = g_normalTexture.Sample(g_sampler, float2(pIn.uv.x * 3, pIn.uv.y * 1.5));
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pIn.normal, pIn.tangentW);
    //bumpedNormalW = normalize(pIn.normal);

    // Diffuse
    float3 dir = normalize(float3(1, 1, 0.0f));
    float diff = max(dot(bumpedNormalW, dir), 0.0);
    float3 diffuse = diff * 1.0;
    diffuse *= float3(.9, .9, 1.0); // Light Color

    // Specular
    float3 I = normalize(pIn.posW - gViewPos);
    float3 R = reflect(I, bumpedNormalW);
    float3 specular = g_skyTexture.Sample(g_sampler, R).rgb;

    float Pi = 6.28318530718f; // Pi*2
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 3.0 - More is better but slower)
    float Size = 0; // BLUR SIZE (Radius)
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
    specular *= 0.05f;

    // Ambient
    float4 aLightColor = float4(1,1,1,1);
    float ambientStrength = 0.2f;
    float3 ambient = ambientStrength * aLightColor;

    float3 result = (ambient + diffuse + specular) * textureColor;

    return float4(result, 1);
}