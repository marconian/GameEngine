#pragma once

#include "CommitedResource.h"
#include "Pipeline.h"
#include "Buffers.h"
#include "Sphere.h"
#include "StepTimer.h"

class PlanetRenderer
{
public:
    PlanetRenderer() noexcept(false);
    PlanetRenderer(const PlanetRenderer& planet) :
        m_environment(planet.m_environment),
        m_graphicInfo(planet.m_graphicInfo),
        m_instanceData(planet.m_instanceData),
        m_vertices(planet.m_vertices),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_instanceBuffer(planet.m_instanceBuffer),
        m_indexBuffer(planet.m_indexBuffer),
        m_pipeline(planet.m_pipeline) { }
    PlanetRenderer& operator=(const PlanetRenderer& planet) = delete;

    void Render(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

    const void UpdateVerticesInput(Sphere::Mesh& mesh);
    const void UpdateInstanceData();

    void CreateDeviceDependentResources();

    struct InstanceData
    {
        unsigned int id;
        DirectX::SimpleMath::Vector3 position;
        float radius;

        struct Material {
            DirectX::SimpleMath::Vector4 color;
            DirectX::SimpleMath::Vector3 Ka;
            DirectX::SimpleMath::Vector3 Kd;
            DirectX::SimpleMath::Vector3 Ks;
            float alpha;
        } material;

        struct Composition {
            float water;
            float soil;
        } composition;
    };

private:
    typedef CommitedResource<DirectX::VertexPositionNormalTexture, D3D12_VERTEX_BUFFER_VIEW> VertexResource;
    typedef CommitedResource<InstanceData, D3D12_VERTEX_BUFFER_VIEW> InstanceResource;
    typedef CommitedResource<uint32_t, D3D12_INDEX_BUFFER_VIEW> IndexResource;

    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;

    Microsoft::WRL::ComPtr<ID3D12Resource>                      m_texture;

    Sphere::Mesh                                                m_graphicInfo;
    std::vector<DirectX::VertexPositionNormalTexture>           m_vertices;
    std::vector<InstanceData>                                   m_instanceData;

    VertexResource                                              m_vertexBuffer;
    InstanceResource                                            m_instanceBuffer;
    IndexResource                                               m_indexBuffer;
    Pipeline                                                    m_pipeline;
};

