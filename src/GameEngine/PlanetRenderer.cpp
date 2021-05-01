#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "CommitedResource.h"
#include "Pipeline.h"
#include "ComputePipeline.h"
#include "TexturePipeline.h"
#include "InputLayout.h"
#include "Sphere.h"
#include "Buffers.h"
#include "SimplexNoise.h"
#include "PlanetRenderer.h"

using namespace std;
using namespace DirectX;
using namespace SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

PlanetRenderer::PlanetRenderer() :
	m_environment(),
	m_system(),
	m_composition(),
	m_colorProfile(),
	m_graphicInfo(Sphere::create(4)),
	m_graphicInfoMedium(Sphere::create(3)),
	m_graphicInfoLow(Sphere::create(2)),
	m_vertices(m_graphicInfo.vertices.size()),
	m_verticesCore(m_graphicInfo.vertices.size()),
	m_verticesMedium(m_graphicInfoMedium.vertices.size()),
	m_verticesLow(m_graphicInfoLow.vertices.size()),
	m_textureData(360 * 360),
	m_instanceBuffer("Instance", g_planets),
	m_vertexBuffer("Vertex", m_vertices),
	m_vertexBufferMedium("VertexMedium", m_verticesMedium),
	m_vertexBufferLow("VertexLow", m_verticesLow),
	m_indexBuffer("Index", m_graphicInfo.indices),
	m_indexBufferCore("IndexCore", m_graphicInfo.indices_p),
	m_indexBufferMedium("IndexMedium", m_graphicInfoMedium.indices),
	m_indexBufferLow("IndexLow", m_graphicInfoLow.indices),
	m_textureBuffer("Texture", m_textureData),
	m_planet(),
	m_planetCore(),
	m_atmosphere(),
	m_distant(),
	m_computeGravity(10240),
	m_computePosition(10240),
	m_computeCollision(10240),
	m_texturePlanet(360)
{
	CreateDeviceDependentResources();
}

void PlanetRenderer::Update(DX::StepTimer const& timer)
{
	float elapsedTime = static_cast<float>(timer.GetElapsedSeconds());
	float time = static_cast<float>(timer.GetTotalSeconds());
	float deltaTime = g_speed * elapsedTime;

	float x = 0;
	float y = 0;
	float z = 0;
	float totalMass = 0;
	float systemMass = 0;
	int noOfPlanets = 0;
	const double massNorm = pow(S_NORM_INV, 3);

	std::vector<Planet*> planets = {};
	g_collisions = 0;

	for (Planet& planet : g_planets)
	{
		if (planet.mass != 0 && !(isnan(planet.position.x) || isnan(planet.position.y) || isnan(planet.position.z)))
		{
			planets.push_back(&planet);

			if (planet.id == g_planets[g_current].id)
				planet.Update(deltaTime);
			//else if (planet.id == m_cursor)
			//	planet.Update(deltaTime * g_planets.size());

			float mass = planet.mass * massNorm;

			x += planet.position.x * mass;
			y += planet.position.y * mass;
			z += planet.position.z * mass;

			totalMass += mass;
			systemMass += planet.mass;
			noOfPlanets++;

			planet.quadrantMass = 0;
			g_collisions += planet.collisions;
		}
	}

	const Vector3 centerOfMass = Vector3(x, y, z) / totalMass;
	for (Planet* planet : planets)
		planet->position -= centerOfMass;


	System system = {};
	system.systemMass = systemMass;
	system.centerOfMass = centerOfMass;
	m_system.Write(&system);

	Environment environment = {};
	environment.deltaTime = elapsedTime * g_speed;
	environment.totalTime = time;
	environment.light = planets[0]->position;
	m_environment.Write(&environment);

	m_composition.Write(&g_compositions[planets[g_current]->id]);

	m_computeGravity.Execute(planets, static_cast<UINT>(planets.size()), static_cast<UINT>(planets.size()));

	std::map<UINT, Planet*> collisions = {};
	std::vector<PlanetDescription> descriptions = {};
	std::vector<PlanetDescription*> descriptionsPtrs = {};

	for (Planet* planet : planets)
	{
		bool const collision = static_cast<bool>(planet->collision);
		if (collision)
		{
			PlanetDescription description{};
			description.planet = *planet;
			description.composition = g_compositions[planet->id];

			collisions[planet->id] = planet;
			descriptions.push_back(description);
		}
	}

	for (PlanetDescription& description : descriptions)
		descriptionsPtrs.push_back(&description);

	m_computeCollision.Execute(descriptionsPtrs, static_cast<UINT>(descriptionsPtrs.size()),
	                           static_cast<UINT>(descriptionsPtrs.size()));

	for (PlanetDescription& description : descriptions)
	{
		memcpy(collisions[description.planet.id], &description.planet, sizeof(Planet));

		Planet& planet = *collisions[description.planet.id];
		if (planet.mass > 0)
		{
			std::vector<DepthInfo>& profile = g_profiles[planet.id];
			size_t const l = profile.size() - static_cast<size_t>(round(profile.size() * .1)) - 1;
			DepthInfo& layer = profile[l];

			float a = g_compositions[description.planet.id].sum();
			float b = description.composition.sum();

			Composition<float> diff = (description.composition - g_compositions[description.planet.id]);
			layer += diff.As<double>();

			double const oldRadius = layer.radius;
			double const usedVolume = l > 0 ? pow(profile[l - 1].radius, 3) * PI_CB : 0;
			double const newRadius = cbrt(((layer.volume + usedVolume) / PI) * (3 / 4.));
			double const radiusChange = newRadius - oldRadius;

			if (radiusChange != 0)
			{
				layer.radius = newRadius;

				if (l < profile.size() - 1)
					for (int i = l + 1; i < profile.size(); i++)
						profile[i].radius += radiusChange;
			}

			memcpy(&g_compositions[description.planet.id], &description.composition, sizeof(Composition<float>));

			planet.RefreshDensityProfile();
		}
		else memcpy(&g_compositions[description.planet.id], &description.composition, sizeof(Composition<float>));
	}

	m_computePosition.Execute(planets, static_cast<UINT>(planets.size()));

	UpdateActivePlanetVerticesColor();

	MoveCursor();
}

void PlanetRenderer::Render(ID3D12GraphicsCommandList* commandList)
{
	PIXBeginEvent(commandList, 0, L"Set vertex and index buffers");

	D3D12_VERTEX_BUFFER_VIEW instanceBuffers[] = {
		m_instanceBuffer.Flush(commandList)
	};
	D3D12_VERTEX_BUFFER_VIEW vertexBuffers[] = {
		m_vertexBufferLow.Flush(commandList),
		m_vertexBufferMedium.Flush(commandList),
		m_vertexBuffer.Flush(commandList)
	};
	D3D12_INDEX_BUFFER_VIEW indexBuffers[] = {
		m_indexBufferLow.Flush(commandList),
		m_indexBufferMedium.Flush(commandList),
		m_indexBuffer.Flush(commandList),
		m_indexBufferCore.Flush(commandList)
	};

	commandList->IASetVertexBuffers(1, 1, instanceBuffers);

	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, 0, L"Draw distant planets");

	if (g_planets.size() > 1)
	{
		std::vector<int> skip{};
		skip.push_back(g_current);

		std::vector<int> medium{};
		for (int i = 0; i < g_planets.size(); i++)
		{
			if (g_planets[i].radius > SUN_DIAMETER * .4)
			{
				skip.push_back(i);
				medium.push_back(i);
			}
		}

		std::sort(skip.begin(), skip.end(), [](int a, int b) { return a < b; });

		m_distant.Execute(commandList);

		commandList->IASetVertexBuffers(0, 1, &vertexBuffers[0]);
		commandList->IASetIndexBuffer(&indexBuffers[0]);

		int start = 0, count = 0, last = 0;
		for (int i : skip)
		{
			count = i - start;
			if (count > 0) commandList->DrawIndexedInstanced(static_cast<UINT>(m_graphicInfoLow.indices.size()), count,
			                                                 0, 0, start);

			start = i + 1;
		}

		count = g_planets.size() - start;
		if (count > 0) commandList->DrawIndexedInstanced(static_cast<UINT>(m_graphicInfoLow.indices.size()), count, 0,
		                                                 0, start);

		commandList->IASetVertexBuffers(0, 1, &vertexBuffers[1]);
		commandList->IASetIndexBuffer(&indexBuffers[1]);

		for (int i : medium)
			if (i != g_current) commandList->DrawIndexedInstanced(static_cast<UINT>(m_graphicInfoMedium.indices.size()),
			                                                      1, 0, 0, i);
	}

	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, 0, L"Draw current planet");
	/*if (!g_coreView)
	{
		commandList->IASetVertexBuffers(0, 1, &vertexBuffers[2]);
		commandList->IASetIndexBuffer(&indexBuffers[2]);

		m_planet.Execute(commandList);

		commandList->DrawIndexedInstanced((UINT)m_graphicInfo.indices.size(), 1, 0, 0, g_current);
	}
	else 
	{
		commandList->IASetVertexBuffers(0, 1, &vertexBuffers[2]);
		commandList->IASetIndexBuffer(&indexBuffers[3]);

		m_planetCore.Execute(commandList);

		commandList->DrawIndexedInstanced((UINT)m_graphicInfo.indices_p.size(), 1, 0, 0, g_current);
	}*/
	commandList->IASetVertexBuffers(0, 1, &vertexBuffers[2]);
	commandList->IASetIndexBuffer(&indexBuffers[2]);

	m_planet.Execute(commandList);

	commandList->DrawIndexedInstanced(static_cast<UINT>(m_graphicInfo.indices.size()), 1, 0, 0, g_current);

	PIXEndEvent(commandList);

	g_device_resources->WaitForGpu();
}

void PlanetRenderer::Refresh()
{
	UpdateActivePlanetVertices();
	UpdateActivePlanetVerticesColor();
}

void PlanetRenderer::UpdateActivePlanetVertices()
{
	Planet const& planet = g_planets[g_current];
	UpdateVertices(m_graphicInfo, m_vertices, 4, &planet);

	if (g_coreView)
	{
		for (VertexPositionNormalColorTexture& vertex : m_vertices)
			if (vertex.position.x > 0) vertex.position.x = 0;
	}
}

void PlanetRenderer::UpdateActivePlanetVerticesColor()
{
	Planet const& planet = g_planets[g_current];

	auto& profile = g_profiles[planet.id];
	double const radiusNorm = profile[profile.size() - 1].radius / 360.;

	double maxDensity = 0, maxPressure = 0;
	for (DepthInfo const& info : profile)
	{
		maxDensity = info.density > maxDensity ? info.density : maxDensity;
		maxPressure = info.pressure > maxPressure ? info.pressure : maxPressure;
	}

	std::array<XMFLOAT4, 180> colorProfile{};
	int j = 0;
	for (int i = 0; i < 180; i++)
	{
		while (profile[j].radius < i * radiusNorm) j++;

		DepthInfo& info = profile[j];
		colorProfile[i] = static_cast<XMFLOAT4>(info.composition.GetColor());
	}

	m_colorProfile.Write(colorProfile.data());

	m_texturePlanet.Execute(360, 360);
}

void PlanetRenderer::UpdateVertices(Sphere::Mesh& mesh,
                                    std::vector<VertexPositionNormalColorTexture>& vertices, const int lod,
                                    const Planet* planet)
{
	vertices.clear();

	mesh = Sphere::create(lod);

	size_t length = mesh.vertices.size();
	XMFLOAT3* normals = new XMFLOAT3[length];
	vector<Vector2> texcoords{};


	const bool applyNoise = planet != nullptr;
	float limit, radius;
	SimplexNoise noise;

	if (applyNoise)
	{
		limit = S_NORM_INV * 100000;
		radius = static_cast<float>(planet->radius * S_NORM_INV);
		noise = SimplexNoise();
	}

	for (Vector3& vertex : mesh.vertices)
	{
		texcoords.push_back(Vector2(
			static_cast<float>(acos(min(max(vertex.x / 1., -1.), 1.)) / PI_RAD * 2),
			static_cast<float>(acos(min(max(vertex.y / 1., -1.), 1.)) / PI_RAD * 2)
		));

		if (applyNoise)
		{
			float const id = sqrt(static_cast<float>(planet->id));
			float noiseVal = noise.fractal(10,
			                               vertex.x + id,
			                               vertex.y + id,
			                               vertex.z + id);

			noiseVal = min(max(noiseVal, -limit), limit);
			vertex *= radius + noiseVal;
		}
	}

	ComputeNormals(mesh.indices.data(), mesh.triangle_count(), mesh.vertices.data(), mesh.vertices.size(), 0, normals);

	for (int i = 0; i < length; i++)
	{
		Vector3 vertex = mesh.vertices[i];
		Vector3 normal = normals[i];
		Vector2 tex = texcoords[i];

		vertices.push_back(VertexPositionNormalColorTexture(vertex, normal, Vector4::Zero, tex));
	}

	texcoords.clear();
	texcoords.shrink_to_fit();
}

void PlanetRenderer::CreateDeviceDependentResources()
{
	InputLayout inputLayout = InputLayout({
		{"SV_POSITION", 0, Vertex, Vector3::Zero},
		{"NORMAL", 0, Vertex, Vector3::Zero},
		{"COLOR", 0, Vertex, Vector4::Zero},
		{"TEXCOORD", 0, Vertex, Vector2::Zero},

		{"INST_ID", 1, Instance, 0},
		{"INST_POSITION", 1, Instance, Vector3::Zero},
		{"INST_DIRECTION", 1, Instance, Vector3::Zero},
		{"INST_VELOCITY", 1, Instance, Vector3::Zero},
		{"INST_ANGULAR", 1, Instance, Vector3::Zero},
		{"INST_RADIUS", 1, Instance, 0.f},
		{"INST_MASS", 1, Instance, 0.f},
		{"INST_TEMP", 1, Instance, 0.f},
		{"INST_DENSITY", 1, Instance, 0.f},
		{"INT_COLLISION", 1, Instance, 0},
		{"INST_COLLISIONS", 1, Instance, 0},
		{"INST_MASS_Q", 1, Instance, 0.f},

		{"INST_MATERIAL_COLOR", 1, Instance, Vector4::Zero},
		{"INST_MATERIAL_KA", 1, Instance, Vector3::Zero},
		{"INST_MATERIAL_KD", 1, Instance, Vector3::Zero},
		{"INST_MATERIAL_KS", 1, Instance, Vector3::Zero},
		{"INST_MATERIAL_ALPHA", 1, Instance, 0.f}
	});

	m_computeGravity.LoadShader("ComputeGravityShader");
	m_computeGravity.SetConstantBuffers({
		m_environment.Description,
		m_system.Description
	});
	m_computeGravity.CreatePipeline();

	m_computeCollision.LoadShader("ComputeCollisionShader");
	m_computeCollision.SetConstantBuffers({
		m_environment.Description
	});
	m_computeCollision.CreatePipeline();

	m_computePosition.LoadShader("ComputePositionShader");
	m_computePosition.SetConstantBuffers({
		m_environment.Description,
		m_system.Description
	});
	m_computePosition.CreatePipeline();

	m_texturePlanet.LoadShader("TextureComputeShader");
	m_texturePlanet.SetConstantBuffers({
		g_settings_buffer->Description,
		m_colorProfile.Description
	});
	m_texturePlanet.CreatePipeline();

	m_planet.set_input_layout(inputLayout);
	m_planet.set_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_planet.load_shaders("SphereVertexShader", "SpherePixelShader");
	m_planet.set_constant_buffers({
		g_mvp_buffer->Description,
		m_environment.Description
	});
	m_planet.set_resource(m_texturePlanet.GetTextureResource());
	m_planet.create_pipeline();

	m_planetCore.set_input_layout(inputLayout);
	m_planetCore.set_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	m_planetCore.load_shaders("CoreVertexShader", "CorePixelShader");
	m_planetCore.set_constant_buffers({
		g_mvp_buffer->Description,
		m_environment.Description
	});
	m_planetCore.set_resource(m_texturePlanet.GetTextureResource());
	m_planetCore.create_pipeline();

	m_atmosphere.set_input_layout(inputLayout);
	m_atmosphere.set_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_atmosphere.load_shaders("AtmosphereVertexShader", "AtmospherePixelShader");
	m_atmosphere.set_constant_buffers({
		g_mvp_buffer->Description,
		m_environment.Description
	});
	m_atmosphere.create_pipeline();


	m_distant.set_input_layout(inputLayout);
	m_distant.set_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_distant.load_shaders("SphereSimpleVertexShader", "SphereSimplePixelShader");
	m_distant.set_constant_buffers({
		g_mvp_buffer->Description,
		m_environment.Description
	});
	m_distant.create_pipeline();

	// Refresh info for sphere mesh.
	UpdateActivePlanetVertices();
	UpdateActivePlanetVerticesColor();
	UpdateVertices(m_graphicInfoMedium, m_verticesMedium, 3);
	UpdateVertices(m_graphicInfoLow, m_verticesLow, 2);
}
