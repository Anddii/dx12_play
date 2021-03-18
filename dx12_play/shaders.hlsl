
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

cbuffer cb_object : register(b0)
{
    float4x4 gWorld;
    float4x4 gViewProj;
    float aspectRatio;
};

struct VertexIn
{
    float4 position : POSITION;
    float4 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint instanceID : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(VertexIn vIn)
{
    PSInput result;

    result.position = mul(vIn.position, gWorld);
    result.position = mul(result.position, gViewProj);
    result.position.x /= aspectRatio;
    result.position.x += vIn.instanceID;
    
    result.uv = float2(vIn.uv.x, -vIn.uv.y);

    result.normal = vIn.normal;

    return result;
}

float4 PSMain(PSInput pIn) : SV_TARGET
{
    float3 lightDir = float3(0.0f,0.5,0.5f);
    //lightDir = normalize(lightDir);

    float3 diff = 2 * lightDir * max(0.0, dot(pIn.normal, lightDir));

    float4 diffuse = g_texture.Sample(g_sampler, pIn.uv);
    float3 finalColor;

    finalColor = diffuse * float4(0.4f, 0.4f, 0.45f, 1.0f); // diffuse * ambient
    finalColor += saturate(dot(diff, pIn.normal) * diffuse * float4(1.0f, 1.0f, 1.0f, 1.0f)); // diffuse * directional

    return float4(finalColor,1.0f);
}