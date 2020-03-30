#pragma once

class ComputePipeline
{
public:
	ComputePipeline();

	void LoadShader(char* compute);
	void SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);

	void CreatePipeline();
	void Execute();

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_cbvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_uavDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>						m_rsc;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>					m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>				m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_commandList;

	D3D12_SHADER_BYTECODE										m_computeShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC>				m_constantBuffers;

	void* m_data;
};

