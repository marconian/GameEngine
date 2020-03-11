#pragma once

template<typename  T, typename  V>
class CommitedResource
{
public:
    CommitedResource(const char* name, std::vector<T>& data);
    CommitedResource(const CommitedResource& self) :
        m_name(self.m_name),
        m_data(self.m_data),
        m_bufferView(self.m_bufferView),
        m_buffer(self.m_buffer),
        m_bufferUpload(self.m_bufferUpload) { }

    const V Flush(ID3D12GraphicsCommandList* commandList);
    const void Release();

private:
    const std::string                               m_name;
    std::vector<T>&                                 m_data;
    V                                               m_bufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_bufferUpload;
};