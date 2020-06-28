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
using namespace DirectX::SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

PlanetRenderer::PlanetRenderer() :
	m_graphicInfo(Sphere::Create(4)),
	m_graphicInfoMedium(Sphere::Create(3)),
	m_graphicInfoLow(Sphere::Create(2)),
	m_environment(),
	m_system(),
	m_composition(),
	m_planet(),
	m_atmosphere(),
	m_distant(),
	m_computeGravity(10240),
	m_computePosition(10240),
	m_computeCollision(10240),
	m_texturePlanet(360),
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
	m_indexBufferMedium("IndexMedium", m_graphicInfoMedium.indices),
	m_indexBufferLow("IndexLow", m_graphicInfoLow.indices),
	m_textureBuffer("Texture", m_textureData)
{
	CreateDeviceDependentResources();
}

void PlanetRenderer::Update(DX::StepTimer const& timer)
{
	float elapsedTime = float(timer.GetElapsedSeconds());
	float time = float(timer.GetTotalSeconds());

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

			if (planet.id == m_cursor)
			{
				planet.Update(timer);

				UpdateActivePlanetVerticesColor();
			}

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
	m_system.Write(system);

	Environment environment = {};
	environment.deltaTime = elapsedTime * g_speed;
	environment.totalTime = time;
	environment.light = planets[0]->position;
	m_environment.Write(environment);

	m_composition.Write(g_compositions[planets[g_current]->id]);

	const bool useQuadrants = planets.size() > 2500;
	if (useQuadrants)
	{
		std::map<std::string, std::vector<Planet*>> quadrants = {};
		std::map<std::string, float> quadrantMasses = {};
		for (Planet* planet : planets)
		{

			const std::string quadrant = planet->GetQuadrant();
			auto quadrantIt = quadrants.find(quadrant);
			if (quadrantIt == quadrants.end())
			{
				quadrants[quadrant] = {};
				quadrantMasses[quadrant] = 0;
			}

			quadrants[quadrant].push_back(planet);
			quadrantMasses[quadrant] += planet->mass;
		}

		UINT quadrantMax = 0;
		for (auto& q : quadrants)
		{
			if (q.second.size() > quadrantMax)
				quadrantMax = q.second.size();
		}

		for (auto& q : quadrants)
		{
			for (auto& p : q.second)
				p->quadrantMass = quadrantMasses[q.first];

			if (q.second.size() > 1)
				m_computeGravity.Execute(q.second, (UINT)q.second.size(), (UINT)q.second.size());
		}


		double quadrantSizeRatio = (double)quadrants.size() / (double)planets.size();
		double quadrantCountRatio = quadrantMax / (double)planets.size();

		if (quadrantSizeRatio < .07 || quadrantCountRatio > .2)
			g_quadrantSize *= .99;
		else if (quadrantSizeRatio > .08)
			g_quadrantSize *= 1.01;
	}
	else
	{
		m_computeGravity.Execute(planets, (UINT)planets.size(), (UINT)planets.size());
	}

	std::map<UINT, Planet*> collisions = {};
	std::vector<PlanetDescription> descriptions = {};
	std::vector<PlanetDescription*> descriptionsPtrs = {};

	for (Planet* planet : planets)
	{
		const bool collision = (bool)planet->collision;
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

	m_computeCollision.Execute(descriptionsPtrs, (UINT)descriptionsPtrs.size(), (UINT)descriptionsPtrs.size());

	for (PlanetDescription& description : descriptions)
	{
		memcpy(collisions[description.planet.id], &description.planet, sizeof(Planet));

		Planet& planet = *collisions[description.planet.id];
		if (planet.mass > 0)
		{
			std::vector<DepthInfo>& profile = g_profiles[planet.id];
			size_t const l = profile.size() - static_cast<size_t>(round(profile.size() * .1)) - 1;
			DepthInfo& layer = profile[l];

			double m = 0;
			for (int i = 0; i < g_compositions[description.planet.id].size(); i++)
			{
				double addedMass = static_cast<double>(description.composition.data()[i]) - static_cast<double>(g_compositions[description.planet.id].data()[i]);
				if (addedMass < 0) addedMass = 0;

				if (addedMass > 0)
				{
					layer.composition.data()[i] += addedMass;
					layer.mass += addedMass;
					layer.volume = layer.mass / layer.density;

					m += addedMass;
				}
			}

			double const usedVolume = l > 0 ? pow(profile[l - 1].radius, 3) * PI_CB : 0;
			double const newRadius = cbrt(((layer.volume + usedVolume) / PI) * (3 / 4.));
			double const radiusChange = newRadius - layer.radius;

			if (radiusChange != 0)
			{
				layer.radius = newRadius;

				if (l < profile.size() - 1)
					for (int i = l + 1; i < profile.size(); i++)
						profile[i].radius += radiusChange;
			}

			memcpy(&g_compositions[description.planet.id], &description.composition, sizeof(Composition<float>));

			planet.RefreshDensityProfile();
			planet.Update(timer);
		}
		else memcpy(&g_compositions[description.planet.id], &description.composition, sizeof(Composition<float>));
	}

	m_computePosition.Execute(planets, (UINT)planets.size());

	m_texturePlanet.Execute(360, 360);

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
		m_indexBuffer.Flush(commandList)
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
			if (count > 0) commandList->DrawIndexedInstanced((UINT)m_graphicInfoLow.indices.size(), count, 0, 0, start);

			start = i + 1;
		}

		count = g_planets.size() - start;
		if (count > 0) commandList->DrawIndexedInstanced((UINT)m_graphicInfoLow.indices.size(), count, 0, 0, start);

		commandList->IASetVertexBuffers(0, 1, &vertexBuffers[1]);
		commandList->IASetIndexBuffer(&indexBuffers[1]);

		for (int i : medium)
			if (i != g_current) commandList->DrawIndexedInstanced((UINT)m_graphicInfoMedium.indices.size(), 1, 0, 0, i);
	}

	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, 0, L"Draw current planet");

	commandList->IASetVertexBuffers(0, 1, &vertexBuffers[2]);
	commandList->IASetIndexBuffer(&indexBuffers[2]);

	m_planet.Execute(commandList);

	commandList->DrawIndexedInstanced((UINT)m_graphicInfo.indices.size(), 1, 0, 0, g_current);

	//m_atmosphere.Execute(commandList);
	//commandList->DrawIndexedInstanced(m_graphicInfo.indices.size(), 1, 0, 0, g_current);

	PIXEndEvent(commandList);

	g_deviceResources->WaitForGpu();
}

void PlanetRenderer::Refresh()
{
	UpdateActivePlanetVertices();
	//UpdateLowResVertices();
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

	UpdateActivePlanetVerticesColor();
}

void PlanetRenderer::UpdateActivePlanetVerticesColor()
{
	Planet const& planet = g_planets[g_current];

	if (g_coreView)
	{
		auto const& profile = g_profiles[planet.id];
		double maxDensity = 0, maxPressure = 0;
		for (DepthInfo const& info : profile)
		{
			maxDensity = info.density > maxDensity ? info.density : maxDensity;
			maxPressure = info.pressure > maxPressure ? info.pressure : maxPressure;
		}

		for (VertexPositionNormalColorTexture& vertex : m_vertices)
		{
			if (vertex.position.x == 0)
			{
				double radius = static_cast<double>(sqrt(pow(vertex.position.z, 2) + pow(vertex.position.y, 2))) * S_NORM;

				double step = abs(profile[0].radius - profile[1].radius);
				size_t pos = static_cast<size_t>(floor(radius / step));
				if (pos > profile.size() - 1)
					pos = profile.size() - 1;

				DepthInfo info = profile[pos];
				//double density = (log10(profile[pos].density) / log10(maxDensity));
				//double pressure = (log10(profile[pos].pressure) / log10(maxPressure));
				//double mass = 1 - (log10(profile[pos].mass) / log10(planet.mass));

				//if (vertex.position.y > 0 && vertex.position.z > 0)
				//    vertex.color = Vector4(density, 0, 0, 1);
				//else vertex.color = info.composition.GetColor();
				//else if (vertex.position.y > 0)
				//    vertex.color = Vector4(pressure, 0, pressure, 1);
				//else vertex.color = Vector4(mass, mass, mass, 1);

				vertex.color = info.composition.GetColor();
			}
		}
	}
}

void PlanetRenderer::UpdateVertices(Sphere::Mesh& mesh, std::vector<DirectX::VertexPositionNormalColorTexture>& vertices, const int lod, const Planet* planet)
{
	vertices.clear();

	mesh = Sphere::Create(lod);

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
		(float)(acos(min(max(vertex.x / 1., -1.), 1.)) / PI_RAD * 2),
			(float)(acos(min(max(vertex.y / 1., -1.), 1.)) / PI_RAD * 2)
			));

		if (applyNoise)
		{
			float noiseVal = noise.fractal(10,
				vertex.x + planet->id,
				vertex.y + planet->id,
				vertex.z + planet->id);

			noiseVal = min(max(noiseVal, -limit), limit);
			vertex *= radius + noiseVal;
		}
	}

	ComputeNormals(mesh.indices.data(), mesh.triangleCount(), mesh.vertices.data(), mesh.vertices.size(), 0, normals);

	for (int i = 0; i < length; i++)
	{
		Vector3 vertex = mesh.vertices[i];
		Vector3 normal = normals[i];
		Vector2 tex = texcoords[i];

		vertices.push_back(DirectX::VertexPositionNormalColorTexture(vertex, normal, Vector4::Zero, tex));
	}

	texcoords.clear();
	texcoords.shrink_to_fit();
}

void PlanetRenderer::CreateDeviceDependentResources()
{
	InputLayout inputLayout = InputLayout({
		{ "SV_POSITION", 0, Vertex, Vector3::Zero },
		{ "NORMAL", 0, Vertex, Vector3::Zero },
		{ "COLOR", 0, Vertex, Vector4::Zero },
		{ "TEXCOORD", 0, Vertex, Vector2::Zero },

		{ "INST_ID", 1, Instance, 0 },
		{ "INST_POSITION", 1, Instance, Vector3::Zero },
		{ "INST_DIRECTION", 1, Instance, Vector3::Zero },
		{ "INST_VELOCITY", 1, Instance, Vector3::Zero },
		{ "INST_ANGULAR", 1, Instance, Vector3::Zero },
		{ "INST_RADIUS", 1, Instance, 0.f },
		{ "INST_MASS", 1, Instance, 0.f },
		{ "INST_TEMP", 1, Instance, 0.f },
		{ "INST_DENSITY", 1, Instance, 0.f },
		{ "INT_COLLISION", 1, Instance, 0 },
		{ "INST_COLLISIONS", 1, Instance, 0 },
		{ "INST_MASS_Q", 1, Instance, 0.f },

		{ "INST_MATERIAL_COLOR", 1, Instance, Vector4::Zero },
		{ "INST_MATERIAL_KA", 1, Instance, Vector3::Zero },
		{ "INST_MATERIAL_KD", 1, Instance, Vector3::Zero },
		{ "INST_MATERIAL_KS", 1, Instance, Vector3::Zero },
		{ "INST_MATERIAL_ALPHA", 1, Instance, 0.f }
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

	});
	m_texturePlanet.CreatePipeline();

	m_planet.SetInputLayout(inputLayout);
	m_planet.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_planet.LoadShaders("SphereVertexShader", "SpherePixelShader");
	m_planet.SetConstantBuffers({
		g_mvp_buffer->Description,
		m_environment.Description
		});
	m_planet.SetResource(m_texturePlanet.GetTextureResource());
	m_planet.CreatePipeline();

	m_atmosphere.SetInputLayout(inputLayout);
	m_atmosphere.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_atmosphere.LoadShaders("AtmosphereVertexShader", "AtmospherePixelShader");
	m_atmosphere.SetConstantBuffers({
		g_mvp_buffer->Description,
		m_environment.Description
		});
	m_atmosphere.CreatePipeline();


	m_distant.SetInputLayout(inputLayout);
	m_distant.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_distant.LoadShaders("SphereSimpleVertexShader", "SphereSimplePixelShader");
	m_distant.SetConstantBuffers({
		g_mvp_buffer->Description,
		m_environment.Description
		});
	m_distant.CreatePipeline();

	// Refresh info for sphere mesh.
	UpdateActivePlanetVertices();
	UpdateVertices(m_graphicInfoMedium, m_verticesMedium, 3);
	UpdateVertices(m_graphicInfoLow, m_verticesLow, 2);
}