#pragma once
#include "shared.h"

#include <wrl/client.h>
#include <d3d12.h>
#include <D3DCompiler.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <stdio.h>
#include <directxmath.h>
#include <exception>
#include "DirectXMesh.h"
#include <math.h>
#include "vertex.h"
#include "texture.h"
#include "utils.h"
#include "mesh.h"

using namespace Microsoft::WRL;
using namespace DirectX;

// Try 
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

class D3D12Motor
{
private:
	// Limit our dispatch threadgroup count to 65536 for indexing simplicity.
	const uint32_t c_maxGroupDispatchCount = 65536u;
	// An integer version of ceil(value / divisor)
	template <typename T, typename U>
	T DivRoundUp(T value, U divisor)
	{
		return (value + divisor - 1) / divisor;
	}

	int frame = 0;

	std::vector<std::shared_ptr<Texture>>m_textures;

	static const UINT FrameCount = 2;

	float m_aspectRatio;

	struct MeshConstantBuffer
	{
		int meshOffset = 0;
	};

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	ComPtr<ID3D12Resource> m_passConstantUpload;

	ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_pipelineStateSky;
	ComPtr<ID3D12GraphicsCommandList6> m_bundle;

	UINT m_rtvDescriptorSize;
	UINT m_cbvSrvDescriptorSize;

	// Synchronization objects.
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	void CreateDepthBuffer();
	void PopulateCommandList();
	void MoveToNextFrame();
	void UpdateViewport(HWND hwnd);

public:

	struct PassConstantBuffer
	{
		XMFLOAT4X4 gViewProj = {};
		XMFLOAT3 gViewPos;
	};
	PassConstantBuffer* m_passConstantBuffer = nullptr;
	DirectX::XMVECTOR m_cameraPosition = DirectX::XMVectorSet(0, 0.0f, 10.0f, 1.0f);
	DirectX::XMVECTOR m_cameraFront = DirectX::XMVectorSet(0, 0.0f, -1.0f, 1.0f);
	DirectX::XMVECTOR m_cameraUp = DirectX::XMVectorSet(0, 1.0f, 0.0f, 1.0f);
	float m_cameraYaw = -90;
	float m_cameraPitch = 0;

	UINT m_frameIndex;
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12GraphicsCommandList6> m_commandList;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];

	std::vector<Mesh> mesh = std::vector<Mesh>();

	D3D12Motor() {}
	~D3D12Motor();
	void LoadPipeline(HWND hwnd);
	void LoadAssets();
	void OnUpdate();
	void OnRender();
	void OnResize(HWND hwnd);
	void WaitForGpu();
	void ThrowIfFailed(HRESULT hr);
};