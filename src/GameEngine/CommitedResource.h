#pragma once

template<typename  T, typename  V>
class CommitedResource
{
public:
    CommitedResource(const char* name, std::vector<T>& data, const size_t width = 0, const size_t height = 0);
    CommitedResource(const CommitedResource& self) :
        m_name(self.m_name),
        m_data(self.m_data),
        m_width(self.m_width),
        m_height(self.m_height),
        m_bufferView(self.m_bufferView),
        m_buffer(self.m_buffer),
        m_bufferUpload(self.m_bufferUpload) { }

    const V Flush(ID3D12GraphicsCommandList* commandList);
    ID3D12Resource* Get() { return m_buffer.Get(); }
    const void Release();

private:
    const std::string                               m_name;
    std::vector<T>&                                 m_data;
    size_t                                          m_width;
    size_t                                          m_height;

    V                                               m_bufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_bufferUpload;
};