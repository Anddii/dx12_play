#include "shared.h"
#define ROOT_SIG "RootConstants(b1, num32bitconstants=3), CBV(b2), SRV(t0), SRV(t1), SRV(t2), SRV(t3), SRV(t4), DescriptorTable(SRV(t5)), DescriptorTable(SRV(t6)), DescriptorTable(SRV(t7)), StaticSampler(s0)"

struct Payload
{
	uint MeshletIndices[32];
};
groupshared Payload s_Payload;

struct Instance
{
	float4x4 gWorld;
	float aspectRatio;
};

cbuffer cb_object : register(b1)
{
	int InstanceCount;
	int InstanceOffset;
	int MeshletCount;
};

cbuffer cbPass : register(b2)
{
	float4x4 gViewProj;
	float3 gViewPos;
};

struct Meshlet 
{
	uint32_t vertexCount;
	uint32_t vertexOffset;
	uint32_t primitiveCount;
	uint32_t primitiveOffset;
};

struct MSVertIn
{
	float3 pos;
	float3 normal;
	float2 uv;
	float3 tangent;
};

struct MSVert
{
	float4 pos : SV_POSITION;
	float3 posW : POSITION0;
	float3 posS : POSITION1;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL0;
	float3 tangentW : TANGENT;
	uint meshletIndex : COLOR0;
};

StructuredBuffer<Meshlet> Meshlets : register(t0);
StructuredBuffer<MSVertIn> Vertices : register(t1);
ByteAddressBuffer UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);
StructuredBuffer<Instance> Instances : register(t4);

float4 TransformPosition(float4 pos, int instanceIndex)
{
	Instance n = Instances[InstanceOffset + instanceIndex];
	float4 positionWS = mul(float4(pos.xyz, 1), n.gWorld);

	positionWS = mul(positionWS, gViewProj);
	positionWS.x /= n.aspectRatio;
	return positionWS;
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
	localIndex = m.vertexOffset + localIndex;

	// Byte address must be 4-byte aligned.
	uint wordOffset = (localIndex & 0x1);
	uint byteOffset = (localIndex / 2) * 4;

	// Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
	uint indexPair = UniqueVertexIndices.Load(byteOffset);
	uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

	return index;
}

uint3 UnpackPrimitive(uint primitive)
{
	// Unpacks a 10 bits per index triangle from a 32-bit uint.
	return uint3(primitive & 0xFF, (primitive >> 10) & 0xFF, (primitive >> 20) & 0xFF);
}

uint3 GetPrimitive(Meshlet m, uint index)
{
	return UnpackPrimitive(PrimitiveIndices[m.primitiveOffset + index]);
}

[RootSignature(ROOT_SIG)]
[NumThreads(32, 1, 1)]
void ASMain(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{
    bool visible = false;

    // Check bounds of meshlet cull data resource
    if (dtid < MeshletCount)
    {
        // Do visibility testing for this thread
        visible = true;
    }

    // Compact visible meshlets into the export payload array
    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dtid / InstanceCount;
    }

    // Dispatch the required number of MS threadgroups to render the visible meshlets
    uint visibleCount = WaveActiveCountBits(visible);
	uint vertico = MeshletCount * InstanceCount;
    DispatchMesh(vertico, 1, 1, s_Payload);
}