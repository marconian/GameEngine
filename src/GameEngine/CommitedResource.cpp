#include "pch.h"

#pragma once
#include "CommitedResource.h"

using namespace std;
using namespace DirectX;
using namespace SimpleMath;


template class CommitedResource<VertexPositionNormalColorTexture, D3D12_VERTEX_BUFFER_VIEW>;
template class CommitedResource<Planet, D3D12_VERTEX_BUFFER_VIEW>;
template class CommitedResource<uint32_t, D3D12_INDEX_BUFFER_VIEW>;
template class CommitedResource<XMFLOAT4, UINT>;

template <typename T, typename V>
CommitedResource<T, V>::CommitedResource(const char* name, vector<T>& data, const size_t width,
                                         const size_t height) :
	m_name(name),
	m_data(data),
	m_width(width),
	m_height(height)
{
	ID3D12Device* device = g_device_resources->GetD3DDevice();
	const unsigned int buffer_size = sizeof(m_data[0]) * m_data.size();

	// create default heap
	CD3DX12_RESOURCE_DESC desc;
	if (m_width > 0 && m_height > 0)
		desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT, width, height, 1, 1, 1, 0);
	else
	{
		desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);
	}

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf()));

	string bufferName = m_name + string(" Buffer Resource Heap");
	m_buffer->SetName(wstring(bufferName.begin(), bufferName.end()).c_str());

	// create upload heap
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_bufferUpload.ReleaseAndGetAddressOf()));

	string bufferUploadName = m_name + string(" Buffer Upload Resource Heap");
	m_bufferUpload->SetName(wstring(bufferUploadName.begin(), bufferUploadName.end()).c_str());
};

template <typename T, typename V>
V CommitedResource<T, V>::Flush(ID3D12GraphicsCommandList* commandList)
{
	unsigned int bufferSize = sizeof(m_data[0]) * m_data.size();

	D3D12_SUBRESOURCE_DATA data{};
	data.pData = m_data.data();
	data.RowPitch = bufferSize;
	data.SlicePitch = bufferSize;

	// we are now creating a command with the command list to copy the data from the upload heap to the default heap
	UpdateSubresources(commandList, m_buffer.Get(), m_bufferUpload.Get(), 0, 0, 1, &data);

	D3D12_RESOURCE_STATES transitionState;
	if (std::is_same<V, D3D12_VERTEX_BUFFER_VIEW>::value)
	{
		transitionState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		D3D12_VERTEX_BUFFER_VIEW bufferView;
		bufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
		bufferView.StrideInBytes = sizeof(m_data[0]);
		bufferView.SizeInBytes = bufferSize;

		m_bufferView = *reinterpret_cast<V*>(&bufferView);
	}
	else if (std::is_same<V, D3D12_INDEX_BUFFER_VIEW>::value)
	{
		transitionState = D3D12_RESOURCE_STATE_INDEX_BUFFER;

		D3D12_INDEX_BUFFER_VIEW bufferView;
		bufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
		bufferView.Format = DXGI_FORMAT_R32_UINT;
		bufferView.SizeInBytes = bufferSize;

		m_bufferView = *reinterpret_cast<V*>(&bufferView);
	}
	else if (std::is_same<V, void>::value)
	{
		transitionState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		m_bufferView = *reinterpret_cast<V*>(nullptr);
	}
	else throw std::invalid_argument("Buffer view type unknown.");

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		                             m_buffer.Get(),
		                             D3D12_RESOURCE_STATE_COPY_DEST,
		                             transitionState
	                             ));

	return m_bufferView;
};

template <typename T, typename V>
void CommitedResource<T, V>::Release()
{
	m_buffer.Reset();
	m_bufferUpload.Reset();
};
