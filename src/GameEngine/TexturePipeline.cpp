#include "pch.h"
#pragma once

#include "ShaderTools.h"
#include "TexturePipeline.h"
#include "Planet.h"

template class TexturePipeline<DirectX::XMFLOAT4>;

template <typename T>
TexturePipeline<T>::TexturePipeline(const size_t size) :
	m_size(size),
	m_fenceValue(0),
	m_computeShader(),
	m_cursor(),
	m_data()
{
	m_constantBuffers.emplace_back(m_cursor.Description);
}

template <typename T>
void TexturePipeline<T>::LoadShader(char* compute)
{
	GetShader(compute, m_computeShader);
}

// Wait for pending GPU work to complete.
template <typename T>
void TexturePipeline<T>::WaitForGpu() noexcept
{
	if (m_commandQueue && m_fence && m_fenceEvent.IsValid())
	{
		// Schedule a Signal command in the GPU queue.
		UINT64 fenceValue = m_fenceValue;
		if (SUCCEEDED(m_commandQueue->Signal(m_fence.Get(), fenceValue)))
		{
			// Wait until the Signal has been processed.
			if (SUCCEEDED(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get())))
			{
				WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);

				// Increment the fence value for the current frame.
				m_fenceValue++;
			}
		}
	}
}

template <typename T>
void TexturePipeline<T>::SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers)
{
	m_constantBuffers.clear();
	m_constantBuffers.shrink_to_fit();

	m_constantBuffers.emplace_back(m_cursor.Description);

	for (auto& buffer : buffers)
		m_constantBuffers.emplace_back(buffer);
}

template <typename T>
void TexturePipeline<T>::CreatePipeline()
{
	auto device = g_device_resources->GetD3DDevice();

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));
	m_commandQueue->SetName(L"TexturePipeline");

	DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
	                                                 IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));
	DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr,
	                                            IID_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
	DX::ThrowIfFailed(m_commandList->Close());

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Create root parameters and initialize first (constants)
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters = std::vector<CD3DX12_ROOT_PARAMETER>(
		m_constantBuffers.size() + 1);

	size_t length = m_constantBuffers.size();
	for (int i = 0; i < length; i++)
		rootParameters[i].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);

	// Include texture and srv
	rootParameters[length].InitAsDescriptorTable(1,
	                                             &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0),
	                                             D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[length + 1].InitAsDescriptorTable(1,
	//    &CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0), D3D12_SHADER_VISIBILITY_ALL);

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root parameter descriptor
	CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};
	rsigDesc.Init(
		rootParameters.size(),
		rootParameters.data(),
		1,
		&samplerDesc,
		rootSignatureFlags);
	DX::ThrowIfFailed(DirectX::CreateRootSignature(device, &rsigDesc, m_rootSignature.ReleaseAndGetAddressOf()));

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
	DX::ThrowIfFailed(
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_cbvDescriptorHeap.ReleaseAndGetAddressOf())));

	m_cbvDescriptorHeap->SetName(L"Constant Buffer View Descriptor Heap");

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = m_cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	device->CreateConstantBufferView(m_constantBuffers.data(), descriptorHandle);

	// Create RSC
	D3D12_RESOURCE_DESC rd{};
	rd.Alignment = 0;
	rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	rd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rd.MipLevels = 1;
	rd.SampleDesc = {1, 0};
	rd.Height = sizeof(T) * m_size;
	rd.Width = sizeof(T) * m_size; // (sizeof(T) * m_size + 0xff) & ~0xff;
	rd.DepthOrArraySize = 1;

	// Create UAV
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
		IID_PPV_ARGS(m_rsc.ReleaseAndGetAddressOf()));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavd{};
	uavd.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	uavd.Texture2D.MipSlice = 0;

	D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc{};
	uavHeapDesc.NumDescriptors = 1;
	uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(
		device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(m_uavDescriptorHeap.ReleaseAndGetAddressOf())));

	m_uavDescriptorHeap->SetName(L"Unordered Access View Descriptor Heap");

	device->CreateUnorderedAccessView(m_rsc.Get(), nullptr, &uavd,
	                                  m_uavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a fence for tracking GPU execution progress.
	DX::ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE,
	                                      IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));
	m_fenceValue++;

	m_fence->SetName(L"TexturePipeline");

	m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
	if (!m_fenceEvent.IsValid())
	{
		throw std::exception("CreateEvent");
	}

	Execute(m_size, m_size);
}

template <typename T>
void TexturePipeline<T>::Execute(const UINT threadX, const UINT threadY, const UINT threadZ)
{
	DX::ThrowIfFailed(m_commandAllocator->Reset());
	DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	m_commandList->SetComputeRootSignature(m_rootSignature.Get());
	m_commandList->SetPipelineState(m_pso.Get());
	m_commandList->SetDescriptorHeaps(1, m_cbvDescriptorHeap.GetAddressOf());

	size_t length = m_constantBuffers.size();
	for (int i = 0; i < length; i++)
	{
		m_commandList->SetComputeRootConstantBufferView(i, m_constantBuffers[i].BufferLocation);
	}

	m_commandList->SetDescriptorHeaps(1, m_uavDescriptorHeap.GetAddressOf());
	m_commandList->SetComputeRootDescriptorTable(length, m_uavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		                               m_rsc.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		                               D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	m_commandList->Dispatch(threadX, threadY, threadZ);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_rsc.Get()));

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		                               m_rsc.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		                               D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	DX::ThrowIfFailed(m_commandList->Close());

	m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));

	WaitForGpu();
}
