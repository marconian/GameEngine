#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "Globals.h"
#include "DeviceResources.h"
#include "Constants.h"
#include "Sphere.h"
#include "ShaderTools.h"
#include "Buffers.h"
#include "Planet.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Buffers;


using Microsoft::WRL::ComPtr;

Planet::Planet(double mass, double size, std::vector<Planet>& planets) :
    //m_device(device),
    m_mass(mass),
    m_size(size),
    m_planets(planets),
    //m_mvp(device),
    m_environment(g_deviceResources->GetD3DDevice()),
    m_material(g_deviceResources->GetD3DDevice()),
    m_position(Vector3::Zero),
    m_origin(Vector3::Zero),
    m_velocity(Vector3::Zero),
    m_rotation(Quaternion::Identity),
    m_color(Colors::White)
{
    CreateDeviceDependentResources();

    Material material = {};
    material.lightColor = Vector4(1.f, 0.f, 0.f, 1.f);
    material.Ka = Vector4(0.0435f, 0.0435f, 0.0435f, 1.f); // Ambient reflectivity
    material.Kd = Vector4(0.1086f, 0.1086f, 0.1086f, 1.f); // Diffuse reflectivity
    material.Ks = Vector4(0.0f, 0.0f, 0.0f, 1.f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    material.shininess = Vector4(0.2f, 0.2f, 0.2f, 1.f);
    m_material.Write(material);
}

//void Planet::Apply() {
//    Matrix _mv = XMMatrixMultiply(g_world, g_camera->View());
//    Matrix _mvp = XMMatrixMultiply(_mv, g_camera->Proj());
//
//    ModelViewProjection mvp = {};
//    mvp.mv = XMMatrixTranspose(_mv);
//    mvp.mvp = XMMatrixTranspose(_mvp);
//    g_mvp_buffer.Write(mvp);
//};

void Planet::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    if (m_position != Vector3::Zero && m_planets.size() > 1) {
        for (Planet p : m_planets) 
        {
            Vector3 p_relative = m_position - p.GetPosition();
            // Calculate Gravity
            const double radius = Vector3::Distance(Vector3::Zero, p_relative);
            if (radius > 0) 
            {
                //const double distance = sqrt(pow(m_velocity.x, 2) + pow(m_velocity.y, 2) + pow(m_velocity.z, 2));
                //double angle = atan(distance / radius);
    
                double g_force = (G_NORMALIZED * p.GetMass() * m_mass) / pow(radius * 1000, 2);
    
                Vector3 g_vector;
                (-p_relative).Normalize(g_vector);
    
                g_vector *= g_force;
                m_velocity += g_vector;
            }
        }
    }

    m_position = Vector3::Lerp(m_position, (m_position + m_velocity), elapsedTime * TIME_DELTA);
    //m_position += m_velocity * TIME_DELTA;

    Environment environment = {};
    environment.light = Vector4(-m_position.x, -m_position.y, -m_position.z, 1.f);
    environment.position = m_position * 2;

    m_environment.Write(environment);
}

void Planet::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    
    WriteToGraphicBuffers(commandList, vertexBufferView, indexBufferView);
    
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    commandList->IASetIndexBuffer(&indexBufferView);

    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer0, g_mvp_buffer->Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer1, m_environment.Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::ConstantBuffer2, m_material.Buffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    commandList->SetPipelineState(m_pso.Get());
    
    PIXBeginEvent(commandList, 0, L"Draw a thin rectangle");
    
    commandList->DrawIndexedInstanced(m_graphicInfo.triangles.size(), 1, 0, 0, 0);
    PIXEndEvent(commandList);
}

void Planet::WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView)
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

void Planet::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);
    if (MSAA_ENABLED)
        rtState.sampleDesc.Count = SAMPLE_COUNT;

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
    m_graphicInfo = Sphere::Create(3);

    vector<VertexPositionNormalColorTexture> vertices = Planet::GetVerticesInput(m_graphicInfo);
    int vBufferSize = sizeof(vertices[0]) * vertices.size();
    int iBufferSize = sizeof(m_graphicInfo.triangles[0]) * m_graphicInfo.triangles.size();

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

    // create default heap to hold index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
        nullptr, // optimized clear value must be null for this type of resource
        IID_PPV_ARGS(m_indexBuffer.GetAddressOf()));

    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    m_indexBuffer->SetName(L"Index Buffer Resource Heap");

    // create upload heap to upload index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(m_indexBufferUpload.GetAddressOf()));
    m_indexBufferUpload->SetName(L"Index Buffer Upload Resource Heap");
}