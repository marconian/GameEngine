#pragma once

#include "CommitedResource.h"
#include "Pipeline.h"
#include "ComputePipeline.h"
#include "TexturePipeline.h"
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
		m_colorProfile(planet.m_colorProfile),
		m_graphicInfo(planet.m_graphicInfo),
		m_graphicInfoMedium(planet.m_graphicInfoMedium),
		m_graphicInfoLow(planet.m_graphicInfoLow),
		m_vertices(planet.m_vertices),
		m_verticesCore(planet.m_verticesCore),
		m_verticesMedium(planet.m_verticesMedium),
		m_verticesLow(planet.m_verticesLow),
		m_textureData(planet.m_textureData),
		m_instanceBuffer(planet.m_instanceBuffer),
		m_vertexBuffer(planet.m_vertexBuffer),
		m_vertexBufferMedium(planet.m_vertexBufferMedium),
		m_vertexBufferLow(planet.m_vertexBufferLow),
		m_indexBuffer(planet.m_indexBuffer),
		m_indexBufferCore(planet.m_indexBufferCore),
		m_indexBufferMedium(planet.m_indexBufferMedium),
		m_indexBufferLow(planet.m_indexBufferLow),
		m_textureBuffer(planet.m_textureBuffer),
		m_planet(planet.m_planet),
		m_planetCore(planet.m_planetCore),
		m_atmosphere(planet.m_atmosphere),
		m_distant(planet.m_distant),
		m_computeGravity(planet.m_computeGravity),
		m_computePosition(planet.m_computePosition),
		m_computeCollision(planet.m_computeCollision),
		m_texturePlanet(planet.m_texturePlanet),
		m_cursor(planet.m_cursor)
	{
	}

	PlanetRenderer& operator=(const PlanetRenderer& planet) = delete;

	void Refresh();

	void Render(ID3D12GraphicsCommandList* commandList);
	void Update(DX::StepTimer const& timer);

private:
	void UpdateVertices(Sphere::Mesh& mesh, std::vector<DirectX::VertexPositionNormalColorTexture>& vertices,
	                    int lod, const Planet* planet = nullptr);
	void UpdateActivePlanetVertices();
	void UpdateActivePlanetVerticesColor();
	void CreateDeviceDependentResources();

	typedef CommitedResource<DirectX::VertexPositionNormalColorTexture, D3D12_VERTEX_BUFFER_VIEW> VertexResource;
	typedef CommitedResource<Planet, D3D12_VERTEX_BUFFER_VIEW> InstanceResource;
	typedef CommitedResource<uint32_t, D3D12_INDEX_BUFFER_VIEW> IndexResource;
	typedef CommitedResource<DirectX::XMFLOAT4, UINT> TextureResource;

	Buffers::ConstantBuffer<Buffers::Environment> m_environment;
	Buffers::ConstantBuffer<Buffers::System> m_system;
	Buffers::ConstantBuffer<Composition<float>> m_composition;
	Buffers::ConstantBuffer<DirectX::XMFLOAT4, 180> m_colorProfile;

	Sphere::Mesh m_graphicInfo;
	Sphere::Mesh m_graphicInfoMedium;
	Sphere::Mesh m_graphicInfoLow;
	std::vector<DirectX::VertexPositionNormalColorTexture> m_vertices;
	std::vector<DirectX::VertexPositionNormalColorTexture> m_verticesCore;
	std::vector<DirectX::VertexPositionNormalColorTexture> m_verticesMedium;
	std::vector<DirectX::VertexPositionNormalColorTexture> m_verticesLow;
	std::vector<DirectX::XMFLOAT4> m_textureData;

	InstanceResource m_instanceBuffer;
	VertexResource m_vertexBuffer;
	VertexResource m_vertexBufferMedium;
	VertexResource m_vertexBufferLow;
	IndexResource m_indexBuffer;
	IndexResource m_indexBufferCore;
	IndexResource m_indexBufferMedium;
	IndexResource m_indexBufferLow;
	TextureResource m_textureBuffer;

	Pipeline m_planet;
	Pipeline m_planetCore;
	Pipeline m_atmosphere;
	Pipeline m_distant;
	ComputePipeline<Planet> m_computeGravity;
	ComputePipeline<Planet> m_computePosition;
	ComputePipeline<PlanetDescription> m_computeCollision;
	TexturePipeline<DirectX::XMFLOAT4> m_texturePlanet;

	uint32_t m_cursor;

	uint32_t MoveCursor()
	{
		const uint32_t limit = static_cast<uint32_t>(g_planets.size());
		uint32_t next = GetPlanetIndex(m_cursor) + 1;

		if (next >= limit)
			next %= limit;

		if (g_planets[next].id != m_cursor)
			m_cursor = g_planets[next].id;

		return m_cursor;
	}
};
