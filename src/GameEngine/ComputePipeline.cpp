#pragma once
#pragma once

#include "pch.h"
#include "ShaderTools.h"
#include "ComputePipeline.h"

const size_t size = 256;
float test[size]{};

ComputePipeline::ComputePipeline()
{

}

void ComputePipeline::LoadShader(char* compute)
{
    GetShader(compute, m_computeShader);
}

void ComputePipeline::SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers)
{
    for (auto& buffer : buffers)
        m_constantBuffers.emplace_back(buffer);
}

void ComputePipeline::CreatePipeline()
{
    auto device = g_deviceResources->GetD3DDevice();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

    DX::ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));

    m_commandQueue->SetName(L"DeviceResources");
    DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, 
        IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));
    DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator.Get(), nullptr,
        IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
    DX::ThrowIfFailed(m_commandList->Close());

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
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0), D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[length + 1].InitAsDescriptorTable(1,
        &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0), D3D12_SHADER_VISIBILITY_ALL);

    // Root parameter descriptor
    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};
    rsigDesc.Init(
        rootParameters.size(),
        rootParameters.data(),
        0,
        nullptr,
        rootSignatureFlags);
    DX::ThrowIfFailed(CreateRootSignature(device, &rsigDesc, m_rootSignature.ReleaseAndGetAddressOf()));

    D3D12_COMPUTE_PIPELINE_STATE_DESC pd{};
    pd.CS = m_computeShader;
    pd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    pd.NodeMask = 0;
    pd.pRootSignature = m_rootSignature.Get();

    DX::ThrowIfFailed(device->CreateComputePipelineState(&pd, IID_PPV_ARGS(m_pso.ReleaseAndGetAddressOf())));

    // Create Heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_cbvDescriptorHeap.ReleaseAndGetAddressOf())));

    m_cbvDescriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = m_cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    device->CreateConstantBufferView(m_constantBuffers.data(), descriptorHandle);

    // Create RSC
    D3D12_HEAP_PROPERTIES prop{};
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    prop.CreationNodeMask = 1;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    prop.Type = D3D12_HEAP_TYPE_CUSTOM;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC rd{};
    rd.Alignment = 0;
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.MipLevels = 1;
    rd.SampleDesc = { 1, 0 };
    rd.Height = 1;
    rd.Width = (sizeof(float) * size + 0xff) & ~0xff;
    rd.DepthOrArraySize = 1;

    auto hr = device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &rd,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(m_rsc.ReleaseAndGetAddressOf()));

    // Create UAV
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavd{};
    uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavd.Format = DXGI_FORMAT_UNKNOWN;
    uavd.Buffer.NumElements = size;
    uavd.Buffer.StructureByteStride = sizeof(float);


    // Create Heap
    D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc{};
    uavHeapDesc.NumDescriptors = 1;
    uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DX::ThrowIfFailed(device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(m_uavDescriptorHeap.ReleaseAndGetAddressOf())));

    m_uavDescriptorHeap->SetName(L"Unordered Access View Descriptor Heap");

    device->CreateUnorderedAccessView(m_rsc.Get(), nullptr, &uavd, m_uavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    //D3D12_RANGE range{ 0, 1 };
    //DX::ThrowIfFailed(m_rsc->Map(0, &range, &m_data));
    DX::ThrowIfFailed(m_rsc->Map(0, nullptr, reinterpret_cast<void**>(&m_data)));
}

void ComputePipeline::Execute()
{
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);

    m_commandList->SetComputeRootSignature(m_rootSignature.Get());
    m_commandList->SetPipelineState(m_pso.Get());
    m_commandList->SetDescriptorHeaps(1, m_cbvDescriptorHeap.GetAddressOf());

    size_t length = m_constantBuffers.size();
    for (int i = 0; i < length; i++)
        m_commandList->SetComputeRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);

    m_commandList->SetDescriptorHeaps(1, m_uavDescriptorHeap.GetAddressOf());
    m_commandList->SetComputeRootDescriptorTable(length, m_uavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->Dispatch(360, 360, 1);

    m_commandList->Close();

    m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));

    ZeroMemory(test, sizeof(test));
    memcpy(test, (float*)m_data, sizeof(test));
}