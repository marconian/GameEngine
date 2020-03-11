#pragma once

#include "InputLayout.h"

class Pipeline
{
public:
	Pipeline();

	void SetInputLayout(InputLayout& layout);
	void SetTopology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology);
	void LoadShaders(char* vertex, char* pixel);
	void SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);

	void CreatePipeline();
	ID3D12PipelineState* Get(ID3D12GraphicsCommandList* commandList);

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_descriptorHeap;

	InputLayout*												m_inputLayout;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE								m_topology;
	D3D12_SHADER_BYTECODE										m_vertexShader;
	D3D12_SHADER_BYTECODE										m_pixelShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC>				m_constantBuffers;
};

