#include "d3d12_motor.h"

// Init Pipeline
// 1. Enable Debug layer
// 2. Create The Device (Device is used to check feature support, and create all other  Direct3D  interface  objects  like  resources,  views,  and  command  lists)
// 3. Describe and create the command queue (The  CPU  submits  commands  to  the  queue  through the Direct3D API using command lists)
// 4. Describe and create the swap chain
// 5. Create descriptor heaps. A descriptor heap can be thought of as an array of descriptors. Where each descriptor fully describes an object to the GPU
    // 5.1 Describe and create a render target view (RTV) descriptor heap.
    // 5.2 Describe and create a shader resource view (SRV) heap for the texture.
    // 5.3 Describe and create a constant buffer view (CBV) heap for the constants.
// 6. Create frame resources
    // 6.1 Create a RTV for each frame
// 7. Create A command allocator, manages the underlying storage for command listsand bundles
// 8. Create A bundle allocator

// Load Assets
// 9. Create an empty root signature.  A The root signature defines the resources the shader programs expect
// 10. Create the pipeline state, which includes compiling and loading shaders
    // 10.1 Compile and load shaders
    // 10.2 Define the vertex input layout
    // 10.3 Describe and create the graphics pipeline state object (PSO)
// 11. Create the command list
// 12. Create the vertex buffer
    // 12.1 Define the geometry for a triangle
    // 12.2 Create Upload heap for the CPU to write
    // 12.3 Create Default heap, the actual buffer resource
    // 12.4 Copy the triangle data to the vertex buffer. Copy data from upload heap to Default heap
    // 12.5 Initialize the vertex buffer view.
// 13. Create and record the bundle
// 14. Create Texture (SRV)
// 15. Create the constant buffer.
// 16. Create synchronization objects and wait until assets have been uploaded to the GPU

void D3D12Motor::LoadPipeline(HWND hwnd) {
#if defined(_DEBUG)
    // 1. Enable the D3D12 debug layer.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

    // 2. Create Device
    ThrowIfFailed(D3D12CreateDevice(
        nullptr, // use Default display adapter 
        D3D_FEATURE_LEVEL_11_0,  
        IID_PPV_ARGS(&m_device)));

    // 3. Describe and create the command queue.    command lists -> command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // 4. Describe and create the swap chain.
    // Define Viewport
    UpdateViewport(hwnd);

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.BufferDesc.Width = (UINT)m_viewport.Width;
    swapChainDesc.BufferDesc.Height = (UINT)m_viewport.Height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain> swapChain;
    ThrowIfFailed(factory->CreateSwapChain(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        &swapChainDesc,
        &swapChain
    ));

    ThrowIfFailed(swapChain.As(&m_swapChain));

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // 5. Create descriptor heaps. A descriptor heap can be thought of as an array of descriptors. Where each descriptor fully describes an object to the GPU.
    {
        // 5.1 Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // 5.2 Describe and create a shader resource view (SRV) heap for the texture(s).
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        // 5.3 Describe and create a constant buffer view (CBV) heap for the constants.
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

        // 5.4 Describe and create a depth stencil view (DSV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = FrameCount;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NodeMask = 0;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    // 6. Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // 6.1 Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            // 7. A command allocator.
            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }

        // 6.2 Create Depth Buffer
        CreateDepthBuffer();
    }
    // 8. A bundle allocator.
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_bundleAllocator)));
}

void D3D12Motor::LoadAssets() {

    // 10. Create the pipeline state, which includes compiling and loading shaders.
    {
        // 9. Create an root signature.
        struct
        {
            byte* data;
            uint32_t size;
        } meshShader, pixelShader;

        ReadDataFromFile(L"MS.cso", &meshShader.data, &meshShader.size);
        ReadDataFromFile(L"PS.cso", &pixelShader.data, &pixelShader.size);

        // Pull root signature from the precompiled mesh shader.
        ThrowIfFailed(m_device->CreateRootSignature(0, meshShader.data, meshShader.size, IID_PPV_ARGS(&m_rootSignature)));

        D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.MS = { meshShader.data, meshShader.size };
        psoDesc.PS = { pixelShader.data, pixelShader.size };
        psoDesc.NumRenderTargets = 2;
        psoDesc.RTVFormats[0] = m_renderTargets[0]->GetDesc().Format;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    // CW front; cull back
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc = DefaultSampleDesc();

        auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
        streamDesc.pPipelineStateSubobjectStream = &psoStream;
        streamDesc.SizeInBytes = sizeof(psoStream);

        ThrowIfFailed(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // 11. Create the command list
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // 15. Create the constant buffer.
    {
        UINT elementByteSize = D3D12Motor::CalcConstantBufferByteSize(sizeof(SceneConstantBuffer));  // CB size is required to be 256-byte aligned.

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(elementByteSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = elementByteSize;
        m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
        memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
    }

    // Transition the resource from its initial state to be used as a depth buffer.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForGpu();
    }
}

void D3D12Motor::CreateDepthBuffer()
{
    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = (UINT)m_viewport.Width;
    depthStencilDesc.Height = (UINT)m_viewport.Height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = false ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = false ? (0 - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12Motor::OnUpdate()
{
    // This is stupid. 
    const float translationSpeed = (frame+0.005f)*0.03f;

    // MOVE TO CAMERA? 
    XMFLOAT4X4 mView = {};
    XMFLOAT4X4 mProj = {};

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 1, 0.1f, 100.0f);
    XMStoreFloat4x4(&mProj, P);

    XMVECTOR pos = XMVectorSet(-3.f, 2.f, 5.f, 1.0f);
    XMVECTOR target = XMVectorSet(-3.f, 0.0f, -5, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMStoreFloat4x4(&mView, view);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
   
    XMStoreFloat4x4(&m_constantBufferData.gViewProj, DirectX::XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&m_constantBufferData.gWorld, XMMatrixRotationRollPitchYaw(0, translationSpeed, 0));
    m_constantBufferData.gAspectRatio = m_viewport.Width / m_viewport.Height;

    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

    frame++;
}

void D3D12Motor::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    MoveToNextFrame();
}

void D3D12Motor::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // Record commands.
    const float clearColor[] = { 0.2f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(2, m_meshletBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(3, m_vertexBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(4, m_indexBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(5, m_primitiveBuffer->GetGPUVirtualAddress());

    int packCount = min(MAX_VERTS / m_modelVertexCount, MAX_PRIMS / m_modelPrimitiveCount);
    float groupsPerInstance = float(m_modelMeshletCount - 1) + 1.0f / packCount;

    uint32_t maxInstancePerBatch = static_cast<uint32_t>(float(c_maxGroupDispatchCount) / groupsPerInstance);
    uint32_t dispatchCount = DivRoundUp(m_instanceCount, maxInstancePerBatch);

    for (uint32_t j = 0; j < dispatchCount; ++j)
    {
        uint32_t instanceOffset = maxInstancePerBatch * j;
        uint32_t instanceCount = min(m_instanceCount - instanceOffset, maxInstancePerBatch);

        m_commandList->SetGraphicsRoot32BitConstant(1, instanceCount, 0);
        m_commandList->SetGraphicsRoot32BitConstant(1, instanceOffset, 1);
        m_commandList->SetGraphicsRoot32BitConstant(1, m_modelMeshletCount, 2);

        uint32_t groupCount = static_cast<uint32_t>(ceilf(groupsPerInstance * instanceCount));
        m_commandList->DispatchMesh(groupCount, 1, 1);
    }

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12Motor::WaitForGpu()
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12Motor::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void D3D12Motor::UpdateViewport(HWND hwnd) {
    // Define Viewport
    RECT rect;
    GetClientRect(hwnd, &rect);
    float fpWidth;
    float fpHeight;
    fpWidth = float(rect.right - rect.left);
    fpHeight = float(rect.bottom - rect.top);

    m_aspectRatio = fpWidth / fpHeight;

    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = 1 * fpWidth;
    m_viewport.Height = 1 * fpHeight;
    m_viewport.MinDepth = 0;
    m_viewport.MaxDepth = 1;

    m_scissorRect.left = static_cast<LONG>(m_viewport.TopLeftX);
    m_scissorRect.right = static_cast<LONG>(m_viewport.TopLeftX + m_viewport.Width);
    m_scissorRect.top = static_cast<LONG>(m_viewport.TopLeftY);
    m_scissorRect.bottom = static_cast<LONG>(m_viewport.TopLeftY + m_viewport.Height);
}

void D3D12Motor::OnResize(HWND hwnd) {
    WaitForGpu();

    UpdateViewport(hwnd);

    // Release Render Target Views (RTV) and reset the frame fence values to the current fence value.
    for (int i = 0; i < FrameCount; ++i) {
        m_renderTargets[i].Reset();
        m_fenceValues[i] = m_fenceValues[m_frameIndex];
    }
    //m_depthStencilBuffer.Reset();

    // Resize the swap chain to the desired dimensions.
    DXGI_SWAP_CHAIN_DESC desc = {};
    m_swapChain->GetDesc(&desc);
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, (UINT)m_viewport.Width, (UINT)m_viewport.Height, desc.BufferDesc.Format, desc.Flags));

    // Resize Render Target Views (RTV)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < FrameCount; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
    }
   
    // Reset the frame index to the current back buffer index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12Motor::CreateVertexBuffer(std::vector<Vertex> verticles, std::vector<Meshlet> meshlets, std::vector<uint8_t> uniqueVertexIB, std::vector<MeshletTriangle> primitiveIndices) {

    m_modelMeshletCount = meshlets.size();
    m_modelVertexCount = meshlets[meshlets.size()-1].VertCount;
    m_modelPrimitiveCount = meshlets[meshlets.size() - 1].PrimCount;

    // 12.1 Define the geometry for a triangle
    const UINT vertexBufferSize = (UINT)verticles.size() * sizeof(Vertex);

    ComPtr<ID3D12Resource> m_uploadVertexBuffer;
    // 12.2 Create Upload heap 
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadVertexBuffer)));

    // 12.3 Create Default heap, the actual buffer resource.
    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

    // 12.4 Copy the triangle data to the vertex buffer. Copy data from upload heap to Default heap
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = verticles.data();
    subResourceData.RowPitch = vertexBufferSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), m_uploadVertexBuffer.Get(), 0, 0, 1, &subResourceData);
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    CD3DX12_RESOURCE_DESC meshletDesc = CD3DX12_RESOURCE_DESC::Buffer(meshlets.size() * sizeof(meshlets[0]));
    CD3DX12_RESOURCE_DESC vertexIndexDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT)uniqueVertexIB.size() * sizeof(std::uint16_t));
    CD3DX12_RESOURCE_DESC primitiveDesc = CD3DX12_RESOURCE_DESC::Buffer(primitiveIndices.size() * sizeof(primitiveIndices[0]));

    CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_meshletBuffer)));
    ThrowIfFailed(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_indexBuffer)));
    ThrowIfFailed(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_primitiveBuffer)));


    ComPtr<ID3D12Resource> m_uploadeshletBuffer;
    ComPtr<ID3D12Resource> m_uploadIndexBuffer;
    ComPtr<ID3D12Resource> m_uploadPrimitiveBuffer;

    CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadeshletBuffer)));
    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadIndexBuffer)));
    ThrowIfFailed(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadPrimitiveBuffer)));

    {
        uint8_t* memory = nullptr;
        m_uploadeshletBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, meshlets.data(), meshlets.size() * sizeof(meshlets[0]));
        m_uploadeshletBuffer->Unmap(0, nullptr);
    }

    {
        uint8_t* memory = nullptr;
        m_uploadIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, uniqueVertexIB.data(), uniqueVertexIB.size());
        m_uploadIndexBuffer->Unmap(0, nullptr);
    }

    {
        uint8_t* memory = nullptr;
        m_uploadPrimitiveBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, primitiveIndices.data(), primitiveIndices.size() * sizeof(primitiveIndices[0]));
        m_uploadPrimitiveBuffer->Unmap(0, nullptr);
    }

    D3D12_RESOURCE_BARRIER postCopyBarriers[3];

    m_commandList->CopyResource(m_indexBuffer.Get(), m_uploadIndexBuffer.Get());
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_commandList->CopyResource(m_meshletBuffer.Get(), m_uploadeshletBuffer.Get());
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_meshletBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    m_commandList->CopyResource(m_primitiveBuffer.Get(), m_uploadPrimitiveBuffer.Get());
    postCopyBarriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_primitiveBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // Close the Commandlist and Execute (Copy data from upload heap to Default heap)
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    WaitForGpu();
}

void D3D12Motor::ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch Win32 API errors.
        throw std::exception();
    }
}

D3D12Motor::~D3D12Motor()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}