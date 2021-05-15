#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <vector>
#include "vertex.h"
#include "DirectXMesh.h"

#include "wavefront_reader.h"
#include "shared.h"
#include "utils.h"

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using namespace Microsoft::WRL;

class Mesh
{
public:
	Mesh::Mesh(std::string fileName, int instanceCount = 1);

	// Model resources.
	ComPtr<ID3D12Resource> m_meshletBuffer;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_primitiveBuffer;

	ComPtr<ID3D12Resource> m_instanceBuffer;
	ComPtr<ID3D12Resource> m_instanceUpload;

	int m_modelVertexCount;
	int m_modelPrimitiveCount;
	int m_modelMeshletCount;
	int m_instanceCount = 1;

	HRESULT InitMesh(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);
	void RegenerateInstances(ID3D12Device* device);

	void SetPosition(int instanceOffset, XMVECTOR position);
	void SetRotation(int instanceOffset, XMVECTOR rotation);
	void SetScale(int instanceOffset, XMVECTOR scale);
	void SetCameraPosition(int instanceOffset, XMVECTOR position);

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_triangleVertices), CEREAL_NVP(m_meshlets), CEREAL_NVP(m_uniqueVertexIB), CEREAL_NVP(m_primitiveIndices));
	}

	std::vector<Vertex> m_triangleVertices;
	std::vector<Meshlet> m_meshlets;
	std::vector<uint8_t> m_uniqueVertexIB;
	std::vector<MeshletTriangle> m_primitiveIndices;

	XMVECTOR m_position = XMVectorSet(0,0,0,0);
	XMVECTOR m_rotation = XMVectorSet(0, 0, 0, 0);
	XMVECTOR m_scale = XMVectorSet(1, 1, 1, 0);

	XMVECTOR m_cameraPosition = XMVectorSet(0, 0, 5.f, 0);

private:

	struct SceneConstantBuffer
	{
		XMFLOAT4X4 gWorld = {};
		XMFLOAT4X4 gViewProj = {};
		float gAspectRatio = 1;
	};

	SceneConstantBuffer* m_instanceData = nullptr;
	
	void UpdateWorld(int instanceOffset);
	void UpdateViewProj(int instanceOffset);
	void ThrowIfFailed(HRESULT hr);

};

