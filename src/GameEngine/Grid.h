#pragma once

class Grid
{
public:
    Grid() noexcept(false);
    ~Grid() { };

    void Render(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

    //void Apply(DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj) {
    //    m_world = world;
    //    m_view = view;
    //    m_proj = proj;
    //};

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

    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    std::unique_ptr<DirectX::BasicEffect>                                   m_effect;

    //DirectX::SimpleMath::Matrix                                             m_world;
    //DirectX::SimpleMath::Matrix                                             m_view;
    //DirectX::SimpleMath::Matrix                                             m_proj;

    DirectX::SimpleMath::Vector3                                            m_origin;
    DirectX::SimpleMath::Quaternion                                         m_rotation;
    size_t                                                                  m_divisions;
    float                                                                   m_cellsize;
    DirectX::XMVECTORF32                                                    m_color;

    //ID3D12Device*                                                           m_device;
};

