#pragma once

#include "InputLayout.h"

class Pipeline
{
public:
	Pipeline();

	void set_input_layout(InputLayout& layout);
	void set_topology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology);
	void load_shaders(char* vertex, char* pixel);
	void set_constant_buffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);
	void set_resource(ID3D12Resource* resource);

	void create_pipeline();
	void Execute(ID3D12GraphicsCommandList* commandList);

private:
	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

	InputLayout* m_inputLayout;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topology;
	D3D12_SHADER_BYTECODE m_vertexShader;
	D3D12_SHADER_BYTECODE m_pixelShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC> m_constantBuffers;

	bool m_hasResource;
	ComPtr<ID3D12Resource> m_resourceBuffer;
};
