#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "Sphere.h"
#include "ShaderTools.h"
#include "Buffers.h"
#include "SimplexNoise.h"
#include "PlanetRenderer.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

PlanetRenderer::PlanetRenderer() :
    m_environment(g_deviceResources->GetD3DDevice()),
    m_material(g_deviceResources->GetD3DDevice()),
    m_vertices()
{
    CreateDeviceDependentResources();

    Material material = {};
    material.lightColor = Colors::White;
    material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
    material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
    material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    material.alpha = 0.f;

    //if (m_mass > (SUN_MASS / M_NORM) * .5)
    //{
    //    material.Ka = Vector3(1);
    //}

    m_material.Write(material);
}

void PlanetRenderer::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    #pragma omp parallel for
    for (Planet& planet : g_planets)
    {
        planet.Update(timer);
    }

    Environment environment = {};
    environment.light = Vector3::Zero;

    m_environment.Write(environment);
}

void PlanetRenderer::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    WriteToGraphicBuffers(commandList);

    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer0, g_mvp_buffer->Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer1, m_environment.Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer2, m_material.Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    commandList->SetPipelineState(m_pso.Get());

    PIXBeginEvent(commandList, 0, L"Draw a thin rectangle");

    commandList->DrawIndexedInstanced(m_graphicInfo.indices.size(), g_planets.size(), 0, 0, 0);
    PIXEndEvent(commandList);
}

const void PlanetRenderer::UpdateVerticesInput(Sphere::Mesh& mesh)
{
    m_vertices.clear();

    size_t length = mesh.vertices.size();
    XMFLOAT3* normals = new XMFLOAT3[length];

    //vector<Vector3> vertices;
    //for (int i = 0; i < length; i++)
    //    vertices.push_back(mesh.vertices[i]) * (GetSize() / 2.));

    ComputeNormals(mesh.indices.data(), mesh.triangleCount(), mesh.vertices.data(), mesh.vertices.size(), 0, normals);

    const double radius = 1;
    SimplexNoise noise = SimplexNoise(rand(0., 1000.), .5f, 1.99f, .5f);

    for (int i = 0; i < length; i++)
    {
        Vector3 vertex = mesh.vertices[i];
        Vector3 normal = normals[i];
        Vector2 tex = Vector2(
            acos(min(max(vertex.x / radius, -1.), 1.)) / PI_RAD * 2,
            acos(min(max(vertex.y / radius, -1.), 1.)) / PI_RAD * 2
        );

        //Vector4 color = m_color;

        float c = noise.fractal(static_cast<int>(5 + std::log(360.f)), tex.x, tex.y);
        Vector4 color = Vector4(
            abs(c),
            abs(c),
            abs(c),
            0.f
        );


        m_vertices.push_back(DirectX::VertexPositionNormalColorTexture(vertex, normal, color, tex));
    }
}

void PlanetRenderer::WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList)
{
    // store vertex buffer in upload heap
    int vBufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = m_vertices.data();
    vertexData.RowPitch = vBufferSize;
    vertexData.SlicePitch = vBufferSize;

    // we are now creating a command with the command list to copy the data from the upload heap to the default heap
    UpdateSubresources(commandList, m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_vertexBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    ));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(m_vertices[0]);
    vertexBufferView.SizeInBytes = vBufferSize;

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)


    // store instance buffer in upload heap
    int nBufferSize = sizeof(m_instanceData[0]) * m_instanceData.size();

    D3D12_SUBRESOURCE_DATA instanceData = {};
    instanceData.pData = m_instanceData.data();
    instanceData.RowPitch = nBufferSize;
    instanceData.SlicePitch = nBufferSize;

    UpdateSubresources(commandList, m_instanceBuffer.Get(), m_instanceBufferUpload.Get(), 0, 0, 1, &instanceData);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_instanceBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    ));

    D3D12_VERTEX_BUFFER_VIEW instanceBufferView;
    instanceBufferView.BufferLocation = m_instanceBuffer->GetGPUVirtualAddress();
    instanceBufferView.StrideInBytes = sizeof(m_instanceData[0]);
    instanceBufferView.SizeInBytes = nBufferSize;

    commandList->IASetVertexBuffers(1, 1, &instanceBufferView);


    // store index buffer in upload heap
    int iBufferSize = sizeof(m_graphicInfo.indices[0]) * m_graphicInfo.indices.size();

    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = &m_graphicInfo.indices[0];
    indexData.RowPitch = iBufferSize;
    indexData.SlicePitch = iBufferSize;

    UpdateSubresources(commandList, m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_indexBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER
    ));

    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView.SizeInBytes = iBufferSize;

    commandList->IASetIndexBuffer(&indexBufferView);
}

void PlanetRenderer::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);
    if (MSAA_ENABLED)
        rtState.sampleDesc.Count = SAMPLE_COUNT;

    //D3D12_INPUT_LAYOUT_DESC inputLayout = VertexPositionNormalColorTexture::InputLayout;
    const D3D12_INPUT_ELEMENT_DESC inputLayoutElements[] = {
        { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "INST_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "INST_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "INST_RADIUS", 0, DXGI_FORMAT_R32_FLOAT, 1, 28, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "INST_TIDAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
    };
    D3D12_INPUT_LAYOUT_DESC inputLayout;
    inputLayout.NumElements = ARRAYSIZE(inputLayoutElements);
    inputLayout.pInputElementDescs = inputLayoutElements;

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
    DX::ThrowIfFailed(CreateRootSignature(device, &rsigDesc, m_rootSignature.GetAddressOf()));

    pd.CreatePipelineState(
        device,
        m_rootSignature.Get(),
        vertexShader,
        pixelShader,
        m_pso.GetAddressOf()
    );

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

    m_descriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[] = {
        g_mvp_buffer->Description,
        m_environment.Description,
        m_material.Description
    };

    device->CreateConstantBufferView(cbvDesc, m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Get info for sphere mesh.
    m_graphicInfo = Sphere::Create(1);
    UpdateVerticesInput(m_graphicInfo);

    #pragma omp parallel for
    for (Planet& planet : g_planets)
    {
        InstanceData item(
            planet.GetPosition(),
            planet.GetColor(),
            planet.GetDiameter() / 2,
            Vector3::Zero
            );

        m_instanceData.push_back(item);
    }

    int vBufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    int nBufferSize = sizeof(InstanceData) * g_planets.size();
    int iBufferSize = sizeof(m_graphicInfo.indices[0]) * m_graphicInfo.indices.size();

    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    device->CreateCommittedResource(
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
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(m_vertexBufferUpload.GetAddressOf()));
    m_vertexBufferUpload->SetName(L"Vertex Buffer Upload Resource Heap");



    // create default heap
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(nBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(m_instanceBuffer.GetAddressOf()));

    m_instanceBuffer->SetName(L"Instance Buffer Resource Heap");

    // create upload heap
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(nBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_instanceBufferUpload.GetAddressOf()));
    m_instanceBufferUpload->SetName(L"Instance Buffer Upload Resource Heap");



    // create default heap to hold index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(m_indexBuffer.GetAddressOf()));

    m_indexBuffer->SetName(L"Index Buffer Resource Heap");

    // create upload heap to upload index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_indexBufferUpload.GetAddressOf()));
    m_indexBufferUpload->SetName(L"Index Buffer Upload Resource Heap");
}