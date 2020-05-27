#pragma once

template<typename  T>
class ComputePipeline
{
public:
	ComputePipeline(const size_t size = 16);
	ComputePipeline(const ComputePipeline& self) :
		m_size(self.m_size),
		m_pso(self.m_pso),
		m_rootSignature(self.m_rootSignature),
		m_cbvDescriptorHeap(self.m_cbvDescriptorHeap),
		m_uavDescriptorHeap(self.m_uavDescriptorHeap),
		m_rsc(self.m_rsc),
		m_commandQueue(self.m_commandQueue),
		m_commandAllocator(self.m_commandAllocator),
		m_commandList(self.m_commandList),
		m_fence(self.m_fence),
		m_fenceValue(self.m_fenceValue),
		m_computeShader(self.m_computeShader),
		m_constantBuffers(self.m_constantBuffers),
		m_data(self.m_data) { }

	void LoadShader(char* compute);
	void SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

	void CreatePipeline();
	void Execute(const std::vector<T>& data, const UINT threadX = 1, const UINT threadY = 1, const UINT threadZ = 1);



private:
	void WaitForGpu() noexcept;

	const size_t												m_size;

	Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_cbvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_uavDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>						m_rsc;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>					m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>				m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>			m_commandList;

	// Presentation fence objects.
	Microsoft::WRL::ComPtr<ID3D12Fence>							m_fence;
	UINT64														m_fenceValue;
	Microsoft::WRL::Wrappers::Event								m_fenceEvent;

	D3D12_SHADER_BYTECODE										m_computeShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC>				m_constantBuffers;

	void* m_data;
};

