#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <D3DCompiler.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <stdio.h>
#include <directxmath.h>
#include <exception>

#include "texture.h"

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

	std::shared_ptr<Texture>m_texture;

	static const UINT FrameCount = 2;

	float m_aspectRatio;

	struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

	struct SceneConstantBuffer
	{
		XMFLOAT4 gWorldViewProj;
		float padding[60]; // Padding so the constant buffer is 256-byte aligned.
	};
	static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[2];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12GraphicsCommandList> m_bundle;

	UINT m_rtvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_defaultBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantBufferData;
	UINT8* m_pCbvDataBegin;


	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
	void UpdateViewport(HWND hwnd);
	void ThrowIfFailed(HRESULT hr);
public:
	D3D12Motor() {}
	~D3D12Motor();
	void LoadPipeline(HWND hwnd);
	void LoadAssets();
	void OnRender();
	void OnResize(HWND hwnd);
};

