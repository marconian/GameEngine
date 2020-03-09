#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"


class PlanetRenderer
{
public:
    PlanetRenderer() noexcept(false);
    PlanetRenderer(const PlanetRenderer& planet) :
        m_descriptorHeap(planet.m_descriptorHeap),
        m_environment(planet.m_environment),
        m_graphicInfo(planet.m_graphicInfo),
        m_indexBuffer(planet.m_indexBuffer),
        m_indexBufferUpload(planet.m_indexBufferUpload),
        m_instanceBuffer(planet.m_instanceBuffer),
        m_instanceBufferUpload(planet.m_instanceBufferUpload),
        m_material(planet.m_material),
        m_pso(planet.m_pso),
        m_rootSignature(planet.m_rootSignature),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_vertexBufferUpload(planet.m_vertexBufferUpload),
        m_vertices(planet.m_vertices) { }
    PlanetRenderer& operator=(const PlanetRenderer& planet) = delete;

    void Render(ID3D12GraphicsCommandList* commandList);
    void WriteToGraphicBuffers(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

    const void UpdateVerticesInput(Sphere::Mesh& mesh);

    void CreateDeviceDependentResources();


private:
    struct InstanceData
    {
        InstanceData(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector4 color, float radius, DirectX::SimpleMath::Vector3 tidal) :
            position(position),
            color(color),
            radius(radius),
            tidal(tidal) { }

        DirectX::SimpleMath::Vector3 position;
        DirectX::SimpleMath::Vector4 color;
        float radius;
        DirectX::SimpleMath::Vector3 tidal;
    };

    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;
    Buffers::ConstantBuffer<Buffers::Material>                  m_material;

    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_instanceBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_vertexBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_instanceBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_indexBufferUpload;

    Sphere::Mesh                                                m_graphicInfo;
    std::vector<DirectX::VertexPositionNormalColorTexture>      m_vertices;
    std::vector<InstanceData>                                   m_instanceData;

    Microsoft::WRL::ComPtr<ID3D12PipelineState>                 m_pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>                 m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>                m_descriptorHeap;

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

