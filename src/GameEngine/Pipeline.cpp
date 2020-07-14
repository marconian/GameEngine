#pragma once

#include "pch.h"
#include "InputLayout.h"
#include "ShaderTools.h"
#include "Pipeline.h"

Pipeline::Pipeline() :
    m_hasResource(false)
{

}

void Pipeline::SetInputLayout(InputLayout& layout)
{
	m_inputLayout = new InputLayout(layout);
}

void Pipeline::SetTopology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology)
{
	m_topology = topology;
}

void Pipeline::LoadShaders(char* vertex, char* pixel)
{
    GetShader(vertex, m_vertexShader);
    GetShader(pixel, m_pixelShader);
}

void Pipeline::SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers)
{
	for (auto& buffer : buffers)
		m_constantBuffers.emplace_back(buffer);
}
void Pipeline::SetResource(ID3D12Resource* resource)
{
    m_resourceBuffer = resource;
    m_hasResource = true;
}

void Pipeline::CreatePipeline()
{
    auto device = g_deviceResources->GetD3DDevice();

    RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);
    if (MSAA_ENABLED)
        rtState.sampleDesc.Count = SAMPLE_COUNT;

    EffectPipelineStateDescription pd(
        &m_inputLayout->GetLayout(),
        CommonStates::AlphaBlend,
        CommonStates::DepthDefault,
        CommonStates::CullNone,
        rtState,
        m_topology);

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // Create root parameters and initialize first (constants)
    size_t length = m_constantBuffers.size();

    std::vector<CD3DX12_ROOT_PARAMETER> rootParameters{};
    for (int i = 0; i < length; i++)
    {
        rootParameters.resize(rootParameters.size() + 1);
        rootParameters[rootParameters.size() - 1].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);
    }

    // Include texture and srv
    //rootParameters[length].InitAsDescriptorTable(1,
    //    &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0), D3D12_SHADER_VISIBILITY_ALL);
    //rootParameters[length + 1].InitAsDescriptorTable(1,
    //    &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0), D3D12_SHADER_VISIBILITY_ALL);

    if (m_hasResource)
    {
        rootParameters.resize(rootParameters.size() + 1);
        rootParameters[rootParameters.size() - 1].InitAsDescriptorTable(1,
            &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0), D3D12_SHADER_VISIBILITY_ALL);

        rootParameters.resize(rootParameters.size() + 1);
        rootParameters[rootParameters.size() - 1].InitAsDescriptorTable(1,
            &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0), D3D12_SHADER_VISIBILITY_ALL);

        //rootParameters[rootParameters.size() - 1].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
    }

    //D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    //samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    //samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    //samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    //samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    //samplerDesc.MipLODBias = 0.0f;
    //samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    //samplerDesc.MinLOD = 0.0f;
    //samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    //samplerDesc.MaxAnisotropy = 0;
    //samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    //samplerDesc.ShaderRegister = 0;
    //samplerDesc.RegisterSpace = 0;
    //samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Root parameter descriptor
    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};
    rsigDesc.Init(
        rootParameters.size(),
        rootParameters.data(),
        0,
        nullptr,
        rootSignatureFlags);
    DX::ThrowIfFailed(CreateRootSignature(device, &rsigDesc, m_rootSignature.GetAddressOf()));

    pd.CreatePipelineState(
        device,
        m_rootSignature.Get(),
        m_vertexShader,
        m_pixelShader,
        m_pso.GetAddressOf()
    );

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = rootParameters.size();// m_hasResource ? 2 : 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

    m_descriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    size_t descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    
    for (auto desc : m_constantBuffers)
    {
        device->CreateConstantBufferView(&desc, descriptorHandle);
        descriptorHandle.Offset(1, descriptorSize);
    }

    if (m_hasResource)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvd{};
        srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        srvd.Texture2D.MipLevels = 1;
        srvd.Texture2D.MostDetailedMip = 0;

        device->CreateShaderResourceView(m_resourceBuffer.Get(), &srvd, descriptorHandle);
        descriptorHandle.Offset(1, descriptorSize);
    }
}

void Pipeline::Execute(ID3D12GraphicsCommandList* commandList)
{
    auto device = g_deviceResources->GetD3DDevice();

    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());

    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pso.Get());

    switch (m_topology)
    {
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH:
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
        break;
    default:
        throw std::invalid_argument("Unsupported primitive topology.");
    }

    size_t descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
    {
        commandList->SetGraphicsRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);
        descriptorHandle.Offset(1, descriptorSize);
    }
    
    if (m_hasResource)
    {
        commandList->SetGraphicsRootDescriptorTable(length, descriptorHandle);
        descriptorHandle.Offset(1, descriptorSize);

        //auto adr = m_resourceBuffer->GetDesc();

        //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        //commandList->SetGraphicsRootShaderResourceView(length, m_resourceBuffer->GetGPUVirtualAddress());
        //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    }
}