#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "CommitedResource.h"
#include "Pipeline.h"
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
    m_environment(),
    m_graphicInfo(Sphere::Create(3)),
    m_vertices(m_graphicInfo.vertices.size()),
    m_instanceData(g_planets.size()),
    m_vertexBuffer("Vertex", m_vertices),
    m_instanceBuffer("Instance", m_instanceData),
    m_indexBuffer("Index", m_graphicInfo.indices),
    m_pipeline()
{
    CreateDeviceDependentResources();
}

void PlanetRenderer::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    #pragma omp parallel for
    for (Planet& planet : g_planets)
        planet.Update(timer);

    UpdateInstanceData();

    Environment environment = {};
    environment.light = Vector3::Zero;
    environment.time = time;

    m_environment.Write(environment);
}

void PlanetRenderer::Render(ID3D12GraphicsCommandList* commandList)
{
    ID3D12PipelineState* pso = m_pipeline.Get(commandList);

    commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer.Flush(commandList));
    commandList->IASetVertexBuffers(1, 1, &m_instanceBuffer.Flush(commandList));
    commandList->IASetIndexBuffer(&m_indexBuffer.Flush(commandList));

    commandList->SetPipelineState(pso);

    PIXBeginEvent(commandList, 0, L"Draw a thin rectangle");

    commandList->DrawIndexedInstanced(m_graphicInfo.indices.size(), g_planets.size(), 0, 0, 0);

    PIXEndEvent(commandList);
}

const void PlanetRenderer::UpdateInstanceData()
{
    m_instanceData.clear();

    for (int i = 0; i < g_planets.size(); i++)
    {
        Planet& planet = g_planets[i];

        InstanceData item;
        item.id = i;
        item.position = planet.GetPosition();
        item.radius = planet.GetDiameter() / 2;

        item.material.color = planet.GetColor();
        item.material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
        item.material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
        item.material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
        item.material.alpha = 0.f;

        if (planet.GetMass() > (SUN_MASS / M_NORM) * .5)
            item.material.Ka = Vector3(1);

        m_instanceData.push_back(item);
    }
}

const void PlanetRenderer::UpdateVerticesInput(Sphere::Mesh& mesh)
{
    m_vertices.clear();

    size_t length = mesh.vertices.size();
    XMFLOAT3* normals = new XMFLOAT3[length];

    ComputeNormals(mesh.indices.data(), mesh.triangleCount(), mesh.vertices.data(), mesh.vertices.size(), 0, normals);

    const double radius = 1;
    SimplexNoise noise = SimplexNoise(rand(0., 1000.), .5f, 1.99f, .5f);

    for (int i = 0; i < length; i++)
    {
        Vector3 vertex = mesh.vertices[i];
        Vector3 normal = normals[i];
        Vector2 tex = Vector2(
            acos(min(max(vertex.x / radius, -1.), 1.)) / PI_RAD * 2,
            acos(min(max(vertex.y / radius, -1.), 1.)) / PI_RAD * 2
        );

        //Vector4 color = m_color;
        //const size_t octaves = 5 + static_cast<int>(std::log(360.));
        //float c = noise.fractal(octaves, tex.x, tex.y);
        //Vector4 color = Vector4(
        //    abs(c),
        //    abs(c),
        //    abs(c),
        //    0.f
        //);

        m_vertices.push_back(DirectX::VertexPositionNormalTexture(vertex, normal, tex));
    }
}

void PlanetRenderer::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    m_pipeline.SetInputLayout(InputLayout({
        { "SV_POSITION", 0, Vertex, Vector3::Zero },
        { "NORMAL", 0, Vertex, Vector3::Zero },
        { "TEXCOORD", 0, Vertex, Vector2::Zero },

        { "INST_ID", 1, Instance, 0 },
        { "INST_POSITION", 1, Instance, Vector3::Zero },
        { "INST_RADIUS", 1, Instance, 0.f },

        { "INST_MATERIAL_COLOR", 1, Instance, Vector4::Zero },
        { "INST_MATERIAL_KA", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_KD", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_KS", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_ALPHA", 1, Instance, 0.f },

        { "INST_COMPOSITION_WATER", 1, Instance, 0.f },
        { "INST_COMPOSITION_SOIL", 1, Instance, 0.f }
    }));
    m_pipeline.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_pipeline.LoadShaders("SphereVertexShader", "SpherePixelShader");
    m_pipeline.SetConstantBuffers({
        g_mvp_buffer->Description,
        m_environment.Description
    });
    m_pipeline.CreatePipeline();


    // Get info for sphere mesh.
    UpdateVerticesInput(m_graphicInfo);

    TexMetadata tex = {};
    tex.width = 360;
    tex.height = 360;
    tex.depth = 1;
    tex.mipLevels = 12;
    tex.dimension = TEX_DIMENSION_TEXTURE2D;
    tex.format = DXGI_FORMAT_R32G32B32_FLOAT;

    DirectX::CreateTexture(device, tex, m_texture.GetAddressOf());
}