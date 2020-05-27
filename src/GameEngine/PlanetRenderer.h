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
        m_graphicInfo(planet.m_graphicInfo),
        m_graphicInfoLow(planet.m_graphicInfoLow),
        m_vertices(planet.m_vertices),
        m_verticesLow(planet.m_verticesLow),
        m_vertexBuffer(planet.m_vertexBuffer),
        m_vertexBufferLow(planet.m_vertexBufferLow),
        m_instanceBuffer(planet.m_instanceBuffer),
        m_indexBuffer(planet.m_indexBuffer),
        m_indexBufferLow(planet.m_indexBufferLow),
        m_planet(planet.m_planet),
        m_atmosphere(planet.m_atmosphere),
        m_distant(planet.m_distant),
        m_computeGravity(planet.m_computeGravity),
        m_computePosition(planet.m_computePosition),
        m_textureBuffer(planet.m_textureBuffer),
        m_textureData(planet.m_textureData) { }
    PlanetRenderer& operator=(const PlanetRenderer& planet) = delete;

    void Render(ID3D12GraphicsCommandList* commandList);
    void Update(DX::StepTimer const& timer);

    const void UpdateVerticesInput(Sphere::Mesh& mesh);

    void CreateDeviceDependentResources();

private:
    typedef CommitedResource<DirectX::VertexPositionNormalTexture, D3D12_VERTEX_BUFFER_VIEW> VertexResource;
    typedef CommitedResource<Planet, D3D12_VERTEX_BUFFER_VIEW> InstanceResource;
    typedef CommitedResource<uint32_t, D3D12_INDEX_BUFFER_VIEW> IndexResource;
    typedef CommitedResource<XMFLOAT4, UINT> TextureResource;

    Buffers::ConstantBuffer<Buffers::Environment>               m_environment;

    Sphere::Mesh                                                m_graphicInfo;
    std::vector<DirectX::VertexPositionNormalTexture>           m_vertices;
    Sphere::Mesh                                                m_graphicInfoLow;
    std::vector<DirectX::VertexPositionNormalTexture>           m_verticesLow;
    std::vector<XMFLOAT4>                                       m_textureData;

    InstanceResource                                            m_instanceBuffer;
    VertexResource                                              m_vertexBuffer;
    VertexResource                                              m_vertexBufferLow;
    IndexResource                                               m_indexBuffer;
    IndexResource                                               m_indexBufferLow;
    TextureResource                                             m_textureBuffer;

    Pipeline                                                    m_planet;
    Pipeline                                                    m_atmosphere;
    Pipeline                                                    m_distant;
    ComputePipeline<Planet>                                     m_computeGravity;
    ComputePipeline<Planet>                                     m_computePosition;
};

