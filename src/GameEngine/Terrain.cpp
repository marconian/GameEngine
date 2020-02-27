#include "pch.h"
#include "Terrain.h"
#include "Sphere.h"
#include "ShaderTools.h"
#include "Buffers.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

Terrain::Terrain(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, bool msaa, int sampleCount) :
    m_device(device),
    m_backBufferFormat(backBufferFormat),
    m_depthBufferFormat(depthBufferFormat),
    m_msaa(msaa),
    m_sampleCount(sampleCount),
    m_origin(Vector3::Zero),
    m_rotation(Quaternion::Identity),
    m_divisions(20),
    m_cellsize(2.f),
    m_color(Colors::White)
{
    CreateDeviceDependentResources();
}

void Terrain::Apply(Matrix proj, Matrix view, Matrix world) {
    m_wvp.world = XMMatrixTranspose(world);
    m_wvp.view = XMMatrixTranspose(view);
    m_wvp.projection = XMMatrixTranspose(proj);

    Update();
};

void Terrain::Update()
{
    m_light.light = Vector4(-500.f, -500.f, -10000.f, 1.f);

    m_material.lightColor = Vector4(1.f, 0.f, 0.f, 1.f);
    m_material.Ka = Vector4(0.0435f, 0.0435f, 0.0435f, 1.f); // Ambient reflectivity
    m_material.Kd = Vector4(0.1086f, 0.1086f, 0.1086f, 1.f); // Diffuse reflectivity
    m_material.Ks = Vector4(0.0f, 0.0f, 0.0f, 1.f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    m_material.shininess = Vector4(0.2f, 0.2f, 0.2f, 1.f);

    memcpy(m_wvpConstantBufferMap, &m_wvp, sizeof(m_wvp));
    memcpy(m_lightConstantBufferMap, &m_light, sizeof(m_light));
    memcpy(m_materialConstantBufferMap, &m_material, sizeof(m_material));

    //DX::ThrowIfFailed(m_wvpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_wvpConstantBufferMap)));
    //DX::ThrowIfFailed(m_lightConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_lightConstantBufferMap)));
    //DX::ThrowIfFailed(m_materialConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_materialConstantBufferMap)));
}

void Terrain::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    
    WriteToGraphicBuffers(commandList, vertexBufferView, indexBufferView);
    
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    commandList->IASetIndexBuffer(&indexBufferView);

    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer0, m_wvpConstantBuffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer1, m_lightConstantBuffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer2, m_materialConstantBuffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    commandList->SetPipelineState(m_pso.Get());
    
    PIXBeginEvent(commandList, 0, L"Draw a thin rectangle");
    
    commandList->DrawIndexedInstanced(m_graphicInfo.triangles.size(), 1, 0, 0, 0);
    PIXEndEvent(commandList);
}

void Terrain::WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView)
{
    vector<VertexPositionNormalColorTexture> vertices = GetVerticesInput(m_graphicInfo);
    int vBufferSize = sizeof(vertices[0]) * vertices.size();

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = vertices.data(); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    UpdateSubresources(commandList, m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_vertexBuffer.Get(), 
        D3D12_RESOURCE_STATE_COPY_DEST, 
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    ));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(vertices[0]);
    vertexBufferView.SizeInBytes = vBufferSize;

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)


    int iBufferSize = sizeof(m_graphicInfo.triangles[0]) * m_graphicInfo.triangles.size();

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = &m_graphicInfo.triangles[0]; // pointer to our index array
    indexData.RowPitch = iBufferSize; // size of all our index buffer
    indexData.SlicePitch = iBufferSize; // also the size of our index buffer

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    UpdateSubresources(commandList, m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_indexBuffer.Get(), 
        D3D12_RESOURCE_STATE_COPY_DEST, 
        D3D12_RESOURCE_STATE_INDEX_BUFFER
    ));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    indexBufferView.SizeInBytes = iBufferSize;

    commandList->IASetIndexBuffer(&indexBufferView);
}

void Terrain::CreateDeviceDependentResources()
{
    RenderTargetState rtState(m_backBufferFormat, m_depthBufferFormat);
    if (m_msaa)
        rtState.sampleDesc.Count = m_sampleCount;

    D3D12_INPUT_LAYOUT_DESC inputLayout = VertexPositionNormalColorTexture::InputLayout;

    EffectPipelineStateDescription pd(
        &inputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        CommonStates::CullNone,
        rtState,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    D3D12_SHADER_BYTECODE vertexShader;
    GetShader("SphereVertexShader", vertexShader);

    D3D12_SHADER_BYTECODE pixelShader;
    GetShader("SpherePixelShader", pixelShader);

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // Create root parameters and initialize first (constants)
    CD3DX12_ROOT_PARAMETER rootParameters[RootParameterIndex::RootParameterCount] = {};
    rootParameters[RootParameterIndex::ConstantBuffer0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameterIndex::ConstantBuffer1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameterIndex::ConstantBuffer2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Include texture and srv
    rootParameters[RootParameterIndex::TextureSRV].InitAsDescriptorTable(1, 
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameterIndex::TextureSampler].InitAsDescriptorTable(1, 
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);

    // Root parameter descriptor
    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};
    rsigDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
    DX::ThrowIfFailed(CreateRootSignature(m_device, &rsigDesc, m_rootSignature.GetAddressOf()));

    pd.CreatePipelineState(
        m_device,
        m_rootSignature.Get(),
        vertexShader,
        pixelShader,
        m_pso.GetAddressOf()
    );

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

    m_descriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    D3D12_HEAP_PROPERTIES uploadHeapProperties;
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProperties.CreationNodeMask = 0;
    uploadHeapProperties.VisibleNodeMask = 0;

    DX::ThrowIfFailed(m_device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(Buffers::WorldViewProjection)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_wvpConstantBuffer.GetAddressOf())));
    DX::ThrowIfFailed(m_device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(Buffers::Light)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_lightConstantBuffer.GetAddressOf())));
    DX::ThrowIfFailed(m_device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(Buffers::Material)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_materialConstantBuffer.GetAddressOf())));
    
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[] = {
        { m_wvpConstantBuffer->GetGPUVirtualAddress(), sizeof(m_wvp) },
        { m_lightConstantBuffer->GetGPUVirtualAddress(), sizeof(m_light) },
        { m_materialConstantBuffer->GetGPUVirtualAddress(), sizeof(m_material) }
    };

    auto cbvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_device->CreateConstantBufferView(cbvDesc, cbvHandle);

    DX::ThrowIfFailed(m_wvpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_wvpConstantBufferMap)));
    DX::ThrowIfFailed(m_lightConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_lightConstantBufferMap)));
    DX::ThrowIfFailed(m_materialConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_materialConstantBufferMap)));

    // Get info for sphere mesh.
    int lod = 2;
    m_graphicInfo = Sphere::Create(lod);

    vector<VertexPositionNormalColorTexture> vertices = Terrain::GetVerticesInput(m_graphicInfo);
    int vBufferSize = sizeof(vertices[0]) * vertices.size();
    int iBufferSize = sizeof(m_graphicInfo.triangles[0]) * m_graphicInfo.triangles.size();

    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(m_vertexBuffer.GetAddressOf()));

    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(m_vertexBufferUpload.GetAddressOf()));
    m_vertexBufferUpload->SetName(L"Vertex Buffer Upload Resource Heap");

    // create default heap to hold index buffer
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
        nullptr, // optimized clear value must be null for this type of resource
        IID_PPV_ARGS(m_indexBuffer.GetAddressOf()));

    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    m_indexBuffer->SetName(L"Index Buffer Resource Heap");

    // create upload heap to upload index buffer
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(m_indexBufferUpload.GetAddressOf()));
    m_indexBufferUpload->SetName(L"Index Buffer Upload Resource Heap");
}