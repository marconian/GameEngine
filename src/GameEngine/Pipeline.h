#pragma once

#include "InputLayout.h"

class Pipeline
{
public:
	Pipeline();

	void SetInputLayout(InputLayout& layout);
	void SetTopology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology);
	void LoadShaders(char* vertex, char* pixel, char* compute = nullptr);
	void SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);
	void SetResource(D3D12_SHADER_RESOURCE_VIEW_DESC description, ID3D12Resource* resource);

	void CreatePipeline();
	void Execute(ID3D12GraphicsCommandList* commandList);

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso_compute;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_descriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_uavDescriptorHeap;

	InputLayout*												m_inputLayout;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE								m_topology;
	D3D12_SHADER_BYTECODE										m_vertexShader;
	D3D12_SHADER_BYTECODE										m_pixelShader;
	D3D12_SHADER_BYTECODE										m_computeShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC>				m_constantBuffers;

	bool														m_hasCompute;
	bool														m_hasResource;
	D3D12_SHADER_RESOURCE_VIEW_DESC								m_resourceDescription;
	Microsoft::WRL::ComPtr<ID3D12Resource>						m_resourceBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>						m_computeBuffer;

	void ExecuteCompute(ID3D12GraphicsCommandList* commandList);
};

