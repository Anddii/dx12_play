
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

Texture2D g_texture : register(t5);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
    uint meshletIndex : COLOR0;
};

float4 PSMain(PSInput pIn) : SV_TARGET
{
    uint MeshletIndex = pIn.meshletIndex;
    float3 oColor = g_texture.Sample(g_sampler, pIn.uv).xyz; // TEXTURE COLOR
    oColor = float3(
        float(MeshletIndex & 1)+0.1,
        float(MeshletIndex & 3) / 4 + 0.1,
        float(MeshletIndex & 7) / 8 + 0.1); // SHOW MESHLETS
    oColor = float3(0.5, 0.5, 0.5); // TEMP COLOR

    // Diffuse
    float3 lightPos = float3(0.0f, 100000.0f, 10000.0f);
    float3 norm = normalize(pIn.normal);
    float3 lightDir = normalize(lightPos - pIn.pos);
    float diff = max(dot(norm, lightDir), 0.0);

    float3 lightColor = float3(0.5, 0.5f, 0.99f);
    float3 diffuse = diff * lightColor;

    // Ambient
    float4 aLightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.5;
    float3 ambient = ambientStrength * aLightColor;

    float3 result = (ambient + diffuse) * oColor;

    return float4(result,1.0f);
}