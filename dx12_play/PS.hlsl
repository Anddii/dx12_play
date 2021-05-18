#define ALPHA_TEST = 1

Texture2D g_texture : register(t5);
SamplerState g_sampler : register(s0);

cbuffer cbPass : register(b2)
{
    float4x4 gViewProj;
    float3 gViewPos;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 posL : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
    uint meshletIndex : COLOR0;
};

float4 PSMain(PSInput pIn) : SV_TARGET
{
    uint MeshletIndex = pIn.meshletIndex;
    //float3 oColor = float3(1.0, 1.0, 1.0); // TEMP COLOR
    // flaot3 oColor = float3(
    //    float(MeshletIndex & 1)+0.1,
    //    float(MeshletIndex & 3) / 4 + 0.1,
    //    float(MeshletIndex & 7) / 8 + 0.1); // SHOW MESHLETS

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
    float3 diffuse = max(0.0, dot(normal, dir)); 
    diffuse *= float3(.5, .5, .5); // Light Color

    // Specular

    // Ambient
    float4 aLightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.2;
    float3 ambient = ambientStrength * aLightColor;

    float3 result = (ambient + diffuse) * textureColor;

    return float4(result, 1);
}