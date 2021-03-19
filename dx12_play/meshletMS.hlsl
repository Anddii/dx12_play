cbuffer cb_object : register(b0)
{
	float4x4 gWorld;
	float4x4 gViewProj;
	float aspectRatio;
};

struct MSvert
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

float4 TransformPosition(float x, float y, float z)
{
	float4 pos = mul(float4(x, y, z, 1), gWorld);
	pos = mul(pos, gViewProj);
	pos.x /= aspectRatio;
	return pos;
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MSMain( 
	out vertices MSvert outVerts[8],
	out indices uint3 outIndices[12])
{

	const uint numVertices = 8;
	const uint numPrimitives = 12;

	//must set the number of primitives and vertices that the Mesh Shader will export.
	SetMeshOutputCounts(numVertices, numPrimitives);

	outVerts[0].pos = TransformPosition(-0.5f, -0.5f, -0.5f);
	outVerts[0].color = float3(0.0f, 0.0f, 0.0f);

	outVerts[1].pos = TransformPosition(-0.5f, -0.5f, 0.5f);
	outVerts[1].color = float3(0.0f, 0.0f, 1.0f);

	outVerts[2].pos = TransformPosition(-0.5f, 0.5f, -0.5f);
	outVerts[2].color = float3(0.0f, 1.0f, 0.0f);

	outVerts[3].pos = TransformPosition(-0.5f, 0.5f, 0.5f);
	outVerts[3].color = float3(0.0f, 1.0f, 1.0f);

	outVerts[4].pos = TransformPosition(0.5f, -0.5f, -0.5f);
	outVerts[4].color = float3(1.0f, 0.0f, 0.0f);

	outVerts[5].pos = TransformPosition(0.5f, -0.5f, 0.5f);
	outVerts[5].color = float3(1.0f, 0.0f, 1.0f);

	outVerts[6].pos = TransformPosition(0.5f, 0.5f, -0.5f);
	outVerts[6].color = float3(1.0f, 1.0f, 0.0f);

	outVerts[7].pos = TransformPosition(0.5f, 0.5f, 0.5f);
	outVerts[6].color = float3(1.0f, 1.0f, 1.0f);

	outIndices[0] = uint3(0, 2, 1);
	outIndices[1] = uint3(1, 2, 3);
	outIndices[2] = uint3(4, 5, 6);
	outIndices[3] = uint3(5, 7, 6);
	outIndices[4] = uint3(0, 1, 5);
	outIndices[5] = uint3(0, 5, 4);
	outIndices[6] = uint3(2, 6, 7);
	outIndices[7] = uint3(2, 7, 3);
	outIndices[8] = uint3(0, 4, 6);
	outIndices[9] = uint3(0, 6, 2);
	outIndices[10] = uint3(1, 3, 7);
	outIndices[11] = uint3(1, 7, 5);

}