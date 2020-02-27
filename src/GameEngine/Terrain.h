#pragma once

#include "Sphere.h"
#include "Buffers.h"

class Terrain
{
public:
    Terrain(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, bool msaa, int sampleCount) noexcept(false);
    ~Terrain() { };

    void Render(ID3D12GraphicsCommandList* commandList);
    void WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView);
    void Update();

    void Apply(DirectX::SimpleMath::Matrix proj, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix world);


    void SetOrigin(DirectX::SimpleMath::Vector3 origin) { m_origin = origin; }
    void SetDivisions(size_t divisions) { m_divisions = divisions; }
    void SetCellSize(float size) { m_cellsize = size; }
    void SetDivisionsAndSize(size_t divisions, float size)
    {
        SetDivisions(divisions);
        SetCellSize(size);
    }

    void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }

    std::vector<DirectX::VertexPositionNormalColorTexture> GetVerticesInput(Sphere::Mesh& mesh)
    {
        std::vector<DirectX::VertexPositionNormalColorTexture> vertices;
        for (DirectX::SimpleMath::Vector3 p : mesh.vertices)
        {
            vertices.push_back(DirectX::VertexPositionNormalColorTexture(
                p, 
                -p * 3, 
                m_color, 
                DirectX::SimpleMath::Vector2(p.x, p.y)
            ));
        }

        return vertices;
    }

    void CreateDeviceDependentResources();

private:
    Buffers::WorldViewProjection                                            m_wvp;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_wvpConstantBuffer;
    UINT8*                                                                  m_wvpConstantBufferMap;

    Buffers::Light                                                          m_light;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_lightConstantBuffer;
    UINT8*                                                                  m_lightConstantBufferMap;

    Buffers::Material                                                       m_material;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_materialConstantBuffer;
    UINT8*                                                                  m_materialConstantBufferMap;

    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_vertexBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_indexBufferUpload;

    Sphere::Mesh                                                            m_graphicInfo;

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

    Microsoft::WRL::ComPtr<ID3D12PipelineState>                             m_pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>                             m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                            m_descriptorHeap;

    // Create root signature.
    enum RootParameterIndex
    {
        ConstantBuffer0,
        ConstantBuffer1,
        ConstantBuffer2,
        TextureSRV,
        TextureSampler,
        RootParameterCount
    };
};


