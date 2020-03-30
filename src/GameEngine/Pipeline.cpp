#pragma once

#include "pch.h"
#include "InputLayout.h"
#include "ShaderTools.h"
#include "Pipeline.h"

Pipeline::Pipeline() :
    m_hasResource(false),
    m_hasCompute(false)
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

void Pipeline::LoadShaders(char* vertex, char* pixel, char* compute)
{
    GetShader(vertex, m_vertexShader);
    GetShader(pixel, m_pixelShader);

    if (compute != nullptr) {
        GetShader(compute, m_computeShader);
        m_hasCompute = true;
    }
}

void Pipeline::SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers)
{
	for (auto& buffer : buffers)
		m_constantBuffers.emplace_back(buffer);
}
void Pipeline::SetResource(D3D12_SHADER_RESOURCE_VIEW_DESC description, ID3D12Resource* resource)
{
    m_resourceDescription = description;
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
    std::vector<CD3DX12_ROOT_PARAMETER> rootParameters = std::vector<CD3DX12_ROOT_PARAMETER>(m_constantBuffers.size() + 3);

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        rootParameters[i].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);

    // Include texture and srv
    rootParameters[length].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0), D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[length + 1].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0), D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[length + 2].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0), D3D12_SHADER_VISIBILITY_ALL);

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

    if (m_hasCompute)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC cpd{};
        cpd.CS = m_computeShader;
        cpd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        cpd.NodeMask = 0;
        cpd.pRootSignature = m_rootSignature.Get();

        DX::ThrowIfFailed(device->CreateComputePipelineState(&cpd, IID_PPV_ARGS(m_pso_compute.ReleaseAndGetAddressOf())));
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descriptorHeap.GetAddressOf())));

    m_descriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    size_t descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    device->CreateConstantBufferView(m_constantBuffers.data(), descriptorHandle);

    // Create UAV
    size_t size = 256;

    device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
        D3D12_HEAP_FLAG_NONE, 
        &CD3DX12_RESOURCE_DESC::Buffer(size * sizeof(float), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
        nullptr,
        IID_PPV_ARGS(m_computeBuffer.ReleaseAndGetAddressOf()));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavd{};
    uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavd.Format = DXGI_FORMAT_UNKNOWN;
    uavd.Buffer.NumElements = size;
    uavd.Buffer.StructureByteStride = sizeof(float);

    descriptorHandle.Offset(descriptorSize);
    device->CreateUnorderedAccessView(m_computeBuffer.Get(), nullptr, &uavd, descriptorHandle);
}

void Pipeline::Execute(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());

    if (m_hasCompute)
        ExecuteCompute(commandList);

    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_computeBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
    //commandList->CopyResource(m_computeBuffer.Get(), m_resourceBuffer.Get()); //Copy the data
    //m_srvDescHeap->SetRootDescriptorTable(1, m_srvDescHeap->GetGPUIncrementHandle(0));

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
    default:
        throw std::invalid_argument("Unsupported primitive topology.");
    }

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        commandList->SetGraphicsRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);
    
    commandList->SetGraphicsRootDescriptorTable(length, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_computeBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void Pipeline::ExecuteCompute(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetComputeRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pso_compute.Get());

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        commandList->SetComputeRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);

    commandList->SetComputeRootDescriptorTable(length, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    commandList->Dispatch(256, 1, 1);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_computeBuffer.Get()));
}