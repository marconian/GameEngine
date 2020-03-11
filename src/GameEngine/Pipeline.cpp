#pragma once

#include "pch.h"
#include "InputLayout.h"
#include "ShaderTools.h"
#include "Pipeline.h"

Pipeline::Pipeline()
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

void Pipeline::CreatePipeline()
{
    auto device = g_deviceResources->GetD3DDevice();

    RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);
    if (MSAA_ENABLED)
        rtState.sampleDesc.Count = SAMPLE_COUNT;

    EffectPipelineStateDescription pd(
        &m_inputLayout->GetLayout(),
        CommonStates::Opaque,
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
    std::vector<CD3DX12_ROOT_PARAMETER> rootParameters = std::vector<CD3DX12_ROOT_PARAMETER>(m_constantBuffers.size() + 2);

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        rootParameters[i].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);

    // Include texture and srv
    rootParameters[length].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[length + 1].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);

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
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

    m_descriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    device->CreateConstantBufferView(m_constantBuffers.data(), m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

ID3D12PipelineState* Pipeline::Get(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());

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
    default:
        throw std::invalid_argument("Unsupported primitive topology.");
    }

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        commandList->SetGraphicsRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);
    
    commandList->SetGraphicsRootDescriptorTable(length, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    return m_pso.Get();
}