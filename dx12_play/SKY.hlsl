TextureCube g_texture : register(t5);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float3 posS : POSITION1;
};

float4 SkyMain(PSInput pIn) : SV_TARGET
{
    float4 textureColor = g_texture.Sample(g_sampler, pIn.posS);
    return float4(textureColor.xyz, 1);
}