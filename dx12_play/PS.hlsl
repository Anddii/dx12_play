
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

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    uint meshletIndex : COLOR0;
};

float4 PSMain(PSInput pIn) : SV_TARGET
{
    uint MeshletIndex = pIn.meshletIndex;
    float3 diffuseColor = float3(
        float(MeshletIndex & 1),
        float(MeshletIndex & 3) / 4,
        float(MeshletIndex & 7) / 8);

    float3 lightDir = float3(0.0f, 0.5, 0.5f);

    float3 diff = 1 * lightDir * max(0.0, dot(pIn.normal, lightDir));

    float3 finalColor;
    finalColor = diffuseColor * float3(0.4f, 0.4f, 0.55f); // diffuse * ambient
    finalColor += saturate(dot(diff, pIn.normal) * float4(diffuseColor, 1.0f) * float4(1.0f, 1.0f, 1.0f, 1.0f)); // diffuse * directional

    return float4(finalColor,1.0f);
}