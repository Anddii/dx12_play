#include "mesh.h"

Mesh::Mesh(std::string fileName, int instanceCount) : m_instanceCount(instanceCount){

    std::ifstream is(fileName, std::ios::binary);

    if (!is) {
        throw std::invalid_argument("received negative value");
        return;
    }

    try {
        cereal::BinaryInputArchive archive(is);
        archive(*this);
    }
    catch (const std::exception& e) {
        OutputDebugStringA(e.what());
        return;
    }
}

HRESULT Mesh::InitMesh(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
    // Populate our command list
    cmdList->Reset(cmdAlloc, nullptr);

    m_modelMeshletCount = (int)m_meshlets.size();
    m_modelVertexCount = m_meshlets[m_meshlets.size() - 1].VertCount;
    m_modelPrimitiveCount = m_meshlets[m_meshlets.size() - 1].PrimCount;

    // 12.1 Define the geometry for a triangle
    const UINT vertexBufferSize = (UINT)m_triangleVertices.size() * sizeof(Vertex);

    ComPtr<ID3D12Resource> m_uploadVertexBuffer;
    // 12.2 Create Upload heap 
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadVertexBuffer)));

    // 12.3 Create Default heap, the actual buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

    // 12.4 Copy the triangle data to the vertex buffer. Copy data from upload heap to Default heap
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = m_triangleVertices.data();
    subResourceData.RowPitch = vertexBufferSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    UpdateSubresources(cmdList, m_vertexBuffer.Get(), m_uploadVertexBuffer.Get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    CD3DX12_RESOURCE_DESC meshletDesc = CD3DX12_RESOURCE_DESC::Buffer(m_meshlets.size() * sizeof(m_meshlets[0]));
    CD3DX12_RESOURCE_DESC vertexIndexDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT)m_uniqueVertexIB.size() * sizeof(std::uint16_t));
    CD3DX12_RESOURCE_DESC primitiveDesc = CD3DX12_RESOURCE_DESC::Buffer(m_primitiveIndices.size() * sizeof(m_primitiveIndices[0]));

    CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_meshletBuffer)));
    ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_indexBuffer)));
    ThrowIfFailed(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_primitiveBuffer)));

    ComPtr<ID3D12Resource> m_uploadeshletBuffer;
    ComPtr<ID3D12Resource> m_uploadIndexBuffer;
    ComPtr<ID3D12Resource> m_uploadPrimitiveBuffer;

    CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadeshletBuffer)));
    ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vertexIndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadIndexBuffer)));
    ThrowIfFailed(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &primitiveDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadPrimitiveBuffer)));

    {
        uint8_t* memory = nullptr;
        m_uploadeshletBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, m_meshlets.data(), m_meshlets.size() * sizeof(m_meshlets[0]));
        m_uploadeshletBuffer->Unmap(0, nullptr);
    }

    {
        uint8_t* memory = nullptr;
        m_uploadIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, m_uniqueVertexIB.data(), m_uniqueVertexIB.size());
        m_uploadIndexBuffer->Unmap(0, nullptr);
    }

    {
        uint8_t* memory = nullptr;
        m_uploadPrimitiveBuffer->Map(0, nullptr, reinterpret_cast<void**>(&memory));
        std::memcpy(memory, m_primitiveIndices.data(), m_primitiveIndices.size() * sizeof(m_primitiveIndices[0]));
        m_uploadPrimitiveBuffer->Unmap(0, nullptr);
    }

    D3D12_RESOURCE_BARRIER postCopyBarriers[3];

    cmdList->CopyResource(m_indexBuffer.Get(), m_uploadIndexBuffer.Get());
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    cmdList->CopyResource(m_meshletBuffer.Get(), m_uploadeshletBuffer.Get());
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_meshletBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    cmdList->CopyResource(m_primitiveBuffer.Get(), m_uploadPrimitiveBuffer.Get());
    postCopyBarriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_primitiveBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    cmdList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);

    // Close the Commandlist and Execute (Copy data from upload heap to Default heap)
    cmdList->Close();
    ID3D12CommandList* ppCommandLists[] = { cmdList };
    cmdQueue->ExecuteCommandLists(1, ppCommandLists);

    // Create our sync fence
    ComPtr<ID3D12Fence> fence;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    cmdQueue->Signal(fence.Get(), 1);

    // Wait for GPU
    if (fence->GetCompletedValue() != 1)
    {
        HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fence->SetEventOnCompletion(1, event);

        WaitForSingleObjectEx(event, INFINITE, false);
        CloseHandle(event);
    }

    RegenerateInstances(device);

    return S_OK;
}

void Mesh::RegenerateInstances(ID3D12Device* device) {

    UINT instanceBufferSize = CalcConstantBufferByteSize(m_instanceCount * sizeof(SceneConstantBuffer));

    const CD3DX12_HEAP_PROPERTIES instanceBufferDefaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    // Create/re-create the instance buffer
    ThrowIfFailed(device->CreateCommittedResource(
        &instanceBufferDefaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &instanceBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_instanceBuffer)
    ));

    const CD3DX12_HEAP_PROPERTIES instanceBufferUploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

    // Create/re-create the instance buffer
    ThrowIfFailed(device->CreateCommittedResource(
        &instanceBufferUploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &instanceBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_instanceUpload)
    ));

    m_instanceUpload->Map(0, nullptr, reinterpret_cast<void**>(&m_instanceData));

    // Regenerate the instances in our scene.
    for (uint32_t i = 0; i < m_instanceCount; ++i)
    {
        auto& inst = m_instanceData[i];

        UpdateWorld(i);
        UpdateViewProj(i);
        inst.gAspectRatio = 1378.00000 / 750.000000;  // HUOMHUOM HUOM TODO 
    }
}

void Mesh::SetPosition(int instanceOffset, XMVECTOR position) {
    auto& inst = m_instanceData[instanceOffset];
    m_position = position;
    UpdateWorld(instanceOffset);
}

void Mesh::SetRotation(int instanceOffset, XMVECTOR rotation) {
    auto& inst = m_instanceData[instanceOffset];
    m_rotation = rotation;
    UpdateWorld(instanceOffset);
}

void Mesh::SetScale(int instanceOffset, XMVECTOR scale) {
    auto& inst = m_instanceData[instanceOffset];
    m_scale = scale;
    UpdateWorld(instanceOffset);
}

void Mesh::UpdateWorld(int instanceOffset) {
    auto& inst = m_instanceData[instanceOffset];
    XMMATRIX world = XMMatrixTranslationFromVector(m_position);
    XMStoreFloat4x4(&inst.gWorld, XMMatrixScalingFromVector(m_scale)* XMMatrixTranspose(world)*XMMatrixRotationRollPitchYawFromVector(m_rotation));
}

void Mesh::SetCameraPosition(int instanceOffset, XMVECTOR position) {
    auto& inst = m_instanceData[instanceOffset];
    m_cameraPosition = position;
    UpdateViewProj(instanceOffset);
}

void Mesh::UpdateViewProj(int instanceOffset) {
    auto& inst = m_instanceData[instanceOffset];
    // MOVE TO CAMERA? 
    XMFLOAT4X4 mView = {};
    XMFLOAT4X4 mProj = {};

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 1, 0.1f, 100.0f);
    XMStoreFloat4x4(&mProj, P);

    XMFLOAT4 v2F;    //the float where we copy the v2 vector members
    XMStoreFloat4(&v2F, m_cameraPosition);   //the function used to copy
    XMVECTOR target = XMVectorSet(v2F.x, v2F.y, -v2F.z, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(m_cameraPosition, target, up);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMStoreFloat4x4(&mView, view);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    XMStoreFloat4x4(&inst.gViewProj, DirectX::XMMatrixTranspose(viewProj));
}

void Mesh::ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch Win32 API errors.
        throw std::exception();
    }
}