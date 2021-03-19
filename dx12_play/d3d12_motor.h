#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <D3DCompiler.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <stdio.h>
#include <directxmath.h>
#include <exception>

#include "vertex.h"
#include "texture.h"
#include "utils.h"

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

	int frame = 0;

	std::shared_ptr<Texture>m_texture;

	static const UINT FrameCount = 2;

	float m_aspectRatio;

	struct SceneConstantBuffer
	{
		XMFLOAT4X4 gWorld = {};
		XMFLOAT4X4 gViewProj = {};
		float gAspectRatio = 1;
	};

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[2];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList6> m_commandList;
	ComPtr<ID3D12GraphicsCommandList6> m_bundle;

	UINT m_rtvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantBufferData;
	UINT8* m_pCbvDataBegin;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	void CreateDepthBuffer();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
	void UpdateViewport(HWND hwnd);
	void ThrowIfFailed(HRESULT hr);

	UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware allocation size (usually 256 bytes).
		return (byteSize + 255) & ~255;
	}

public:
	D3D12Motor() {}
	~D3D12Motor();
	void LoadPipeline(HWND hwnd);
	void LoadAssets();
	void CreateVertexBuffer(std::vector<Vertex> verticles);
	void CreateIndexBuffer(std::vector<std::uint16_t> indices);
	void OnUpdate();
	void OnRender();
	void OnResize(HWND hwnd);
};

