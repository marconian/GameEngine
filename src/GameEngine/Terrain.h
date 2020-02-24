#pragma once

class Terrain
{
public:
    Terrain(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, bool msaa, int sampleCount) noexcept(false);
    ~Terrain() { };

    void Render(ID3D12GraphicsCommandList* commandList);
    void Update();

    void Apply(DirectX::SimpleMath::Matrix proj, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world) {
        m_proj = proj;
        m_view = view;
        m_world = world;
    };

    void SetOrigin(DirectX::SimpleMath::Vector3 origin) { m_origin = origin; }
    void SetDivisions(size_t divisions) { m_divisions = divisions; }
    void SetCellSize(float size) { m_cellsize = size; }
    void SetDivisionsAndSize(size_t divisions, float size)
    {
        SetDivisions(divisions);
        SetCellSize(size);
    }

    void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }

    void CreateDeviceDependentResources();

private:

    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionNormalColor>>  m_batch;
    std::unique_ptr<DirectX::BasicEffect>                                   m_effect;

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_proj;

    DirectX::SimpleMath::Vector3                                            m_origin;
    DirectX::SimpleMath::Quaternion                                         m_rotation;
    size_t                                                                  m_divisions;
    float                                                                   m_cellsize;
    DirectX::XMVECTORF32                                                    m_color;


    DXGI_FORMAT                                                             m_backBufferFormat;
    DXGI_FORMAT                                                             m_depthBufferFormat;

    ID3D12Device*                                                           m_device;
    unsigned int                                                            m_sampleCount;
    bool                                                                    m_msaa;

    Microsoft::WRL::ComPtr<ID3D12RootSignature>                             m_rootSignature;

    // Create root signature.
    enum RootParameterIndex
    {
        ConstantBuffer,
        TextureSRV,
        TextureSampler,
        RootParameterCount
    };
};


