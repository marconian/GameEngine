#pragma once

#include "CommitedResource.h"
#include "Pipeline.h"
#include "ComputePipeline.h"
#include "Buffers.h"
#include "Sphere.h"
#include "StepTimer.h"

class PlanetRenderer
{
public:
    PlanetRenderer() noexcept(false);
    PlanetRenderer(const PlanetRenderer& planet) :
        m_environment(planet.m_environment),
        m_system(planet.m_system),
        m_composition(planet.m_composition),
        m_graphicInfo(planet.m_graphicInfo),
        m_graphicInfoMedium(planet.m_graphicInfoMedium),
        m_graphicInfoLow(planet.m_graphicInfoLow),
        m_vertices(planet.m_vertices),
        m_verticesMedium(planet.m_verticesMedium),
        m_verticesLow(planet.m_verticesLow),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_vertexBufferMedium(planet.m_vertexBufferMedium),
        m_vertexBufferLow(planet.m_vertexBufferLow),
        m_instanceBuffer(planet.m_instanceBuffer),
        m_indexBuffer(planet.m_indexBuffer),
        m_indexBufferMedium(planet.m_indexBufferMedium),
        m_indexBufferLow(planet.m_indexBufferLow),
        m_planet(planet.m_planet),
        m_atmosphere(planet.m_atmosphere),
        m_distant(planet.m_distant),
        m_computeGravity(planet.m_computeGravity),
        m_computePosition(planet.m_computePosition),
        m_computeCollision(planet.m_computeCollision),
        m_textureBuffer(planet.m_textureBuffer),
        m_textureData(planet.m_textureData),
        m_cursor(planet.m_cursor) { }
    PlanetRenderer& operator=(const PlanetRenderer& planet) = delete;

    void Refresh();

    void Render(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

private:
    void UpdateVertices(Sphere::Mesh& mesh, std::vector<DirectX::VertexPositionNormalTexture>& vertices, const int lod, const Planet* planet = nullptr);
    void UpdateActivePlanetVertices();
    void CreateDeviceDependentResources();

    typedef CommitedResource<DirectX::VertexPositionNormalTexture, D3D12_VERTEX_BUFFER_VIEW> VertexResource;
    typedef CommitedResource<Planet, D3D12_VERTEX_BUFFER_VIEW> InstanceResource;
    typedef CommitedResource<uint32_t, D3D12_INDEX_BUFFER_VIEW> IndexResource;
    typedef CommitedResource<XMFLOAT4, UINT> TextureResource;

    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;
    Buffers::ConstantBuffer<Buffers::System>                    m_system;
    Buffers::ConstantBuffer<Composition>                        m_composition;

    Sphere::Mesh                                                m_graphicInfo;
    Sphere::Mesh                                                m_graphicInfoMedium;
    Sphere::Mesh                                                m_graphicInfoLow;
    std::vector<DirectX::VertexPositionNormalTexture>           m_vertices;
    std::vector<DirectX::VertexPositionNormalTexture>           m_verticesMedium;
    std::vector<DirectX::VertexPositionNormalTexture>           m_verticesLow;
    std::vector<XMFLOAT4>                                       m_textureData;

    InstanceResource                                            m_instanceBuffer;
    VertexResource                                              m_vertexBuffer;
    VertexResource                                              m_vertexBufferMedium;
    VertexResource                                              m_vertexBufferLow;
    IndexResource                                               m_indexBuffer;
    IndexResource                                               m_indexBufferMedium;
    IndexResource                                               m_indexBufferLow;
    TextureResource                                             m_textureBuffer;

    Pipeline                                                    m_planet;
    Pipeline                                                    m_atmosphere;
    Pipeline                                                    m_distant;
    ComputePipeline<Planet>                                     m_computeGravity;
    ComputePipeline<Planet>                                     m_computePosition;
    ComputePipeline<PlanetDescription>                          m_computeCollision;

    uint32_t                                                    m_cursor;

    uint32_t MoveCursor() {
        const uint32_t limit = static_cast<uint32_t>(g_planets.size());
        uint32_t next = GetPlanetIndex(m_cursor) + 1;

        if (next >= limit)
            next %= limit;

        m_cursor = g_planets[next].id;

        return m_cursor;
    }
};

