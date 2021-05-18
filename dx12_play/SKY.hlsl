TextureCube g_texture : register(t5);
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

float4 SkyMain(PSInput pIn) : SV_TARGET
{
    float4 textureColor = g_texture.Sample(g_sampler, pIn.posL);
    return float4(textureColor.xyz, 1);
}