#pragma once

template <typename T>
class TexturePipeline
{
public:
	TexturePipeline(size_t size = 16);

	TexturePipeline(const TexturePipeline& self) :
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
		m_data(self.m_data)
	{
	}

	void LoadShader(char* compute);
	void SetConstantBuffers(std::initializer_list<D3D12_CONSTANT_BUFFER_VIEW_DESC> buffers);
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

	ID3D12Resource* GetTextureResource() { return m_rsc.Get(); }

	void CreatePipeline();
	void Execute(UINT threadX = 1, UINT threadY = 1, UINT threadZ = 1);

private:
	void WaitForGpu() noexcept;

	const size_t m_size;

	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_cbvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> m_uavDescriptorHeap;
	ComPtr<ID3D12Resource> m_rsc;

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	// Presentation fence objects.
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	Microsoft::WRL::Wrappers::Event m_fenceEvent;

	D3D12_SHADER_BYTECODE m_computeShader;
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC> m_constantBuffers;
	Buffers::ConstantBuffer<uint32_t> m_cursor;

	void* m_data;
};
