#include "shared.h"
#define ROOT_SIG "CBV(b0), RootConstants(b1, num32bitconstants=3), SRV(t0), SRV(t1), SRV(t2), SRV(t3)"

cbuffer cb_object : register(b0)
{
	float4x4 gWorld;
	float4x4 gViewProj;
	float aspectRatio;
};

cbuffer cb_object : register(b1)
{
	int InstanceCount;
	int InstanceOffset;
	int MeshletCount;
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
	float3 color;
};

struct MSVert
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	uint meshletIndex : COLOR0;
};

float4 TransformPosition(float4 pos)
{
	pos.w = 1.0f;
	pos = mul(pos, gWorld);
	pos = mul(pos, gViewProj);
	pos.x /= aspectRatio;
	return pos;
}

StructuredBuffer<Meshlet> Meshlets : register(t0);
StructuredBuffer<MSVertIn> Vertices : register(t1);
ByteAddressBuffer UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);

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
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void MSMain(
	in uint groupId : SV_GroupID,
	in uint groupThreadId : SV_GroupThreadID,
	out vertices MSVert outVerts[MAX_VERTS],
	out indices uint3 outIndices[MAX_PRIMS])
{

	uint meshletIndex = groupId / InstanceCount;
	Meshlet meshlet = Meshlets[meshletIndex];

	uint startInstance = groupId % InstanceCount;
	uint instanceCount = 1;

	// Last meshlet in mesh may be be packed - multiple instances submitted by a single threadgroup.
	if (meshletIndex == MeshletCount - 1)
	{
		const uint instancesPerGroup = min(MAX_VERTS / meshlet.vertexCount, MAX_PRIMS / meshlet.primitiveCount);

		// Determine how many packed instances there are in this group
		uint unpackedGroupCount = (MeshletCount - 1) * InstanceCount;
		uint packedIndex = groupId - unpackedGroupCount;

		startInstance = packedIndex * instancesPerGroup;
		instanceCount = min(InstanceCount - startInstance, instancesPerGroup);
	}

	const uint vertCount = meshlet.vertexCount * instanceCount;
	const uint primCount = meshlet.primitiveCount * instanceCount;

	SetMeshOutputCounts(vertCount, primCount);
	
	if (groupThreadId < vertCount)
	{

		uint readIndex = groupThreadId % meshlet.vertexCount;  // Wrap our reads for packed instancing.
		uint instanceId = groupThreadId / meshlet.vertexCount; // Instance index into this threadgroup's instances (only non-zero for packed threadgroups.)

		uint vertexIndex = GetVertexIndex(meshlet, readIndex);
		uint instanceIndex = startInstance + instanceId;	

		float4 pos = float4(Vertices[vertexIndex].pos.xyz, 1.0f);

		pos.z += -1.0f * round(instanceIndex / 9);
		pos = TransformPosition(pos);
		//pos.x += 1 * instanceIndex;
		pos.x += 1 * instanceIndex % 9;

		outVerts[groupThreadId].pos = pos;
		outVerts[groupThreadId].normal = Vertices[vertexIndex].color;
		outVerts[groupThreadId].meshletIndex = meshletIndex;
	}

	if (groupThreadId < primCount)
	{
		uint readIndex = groupThreadId % meshlet.primitiveCount;  // Wrap our reads for packed instancing.
		uint instanceId = groupThreadId / meshlet.primitiveCount; // Instance index within this threadgroup (only non-zero in last meshlet threadgroups.)
		//outIndices[groupThreadId] = cubeIndices[groupThreadId];
		outIndices[groupThreadId] = GetPrimitive(meshlet, readIndex)+(meshlet.vertexCount * instanceId);
	}
}