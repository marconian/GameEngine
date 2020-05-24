#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "CommitedResource.h"
#include "Pipeline.h"
#include "ComputePipeline.h"
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
    m_graphicInfoLow(Sphere::Create(1)),
    m_environment(),
    m_vertices(m_graphicInfo.vertices.size()),
    m_verticesLow(m_graphicInfoLow.vertices.size()),
    m_instanceData(g_planets.size()),
    m_textureData(360 * 360),
    m_instanceBuffer("Instance", m_instanceData),
    m_vertexBuffer("Vertex", m_vertices),
    m_indexBuffer("Index", m_graphicInfo.indices),
    m_vertexBufferLow("VertexLow", m_verticesLow),
    m_indexBufferLow("IndexLow", m_graphicInfoLow.indices),
    m_textureBuffer("Texture", m_textureData),
    m_planet(),
    m_atmosphere(),
    m_distant(),
    m_compute(16)
{
    CreateDeviceDependentResources();
}

void PlanetRenderer::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    UpdateInstanceData();

    Environment environment = {};
    environment.light = Vector3::Zero;
    environment.time = time;

    m_environment.Write(environment);

    Planet::PlanetDescription output[16]{};
    memcpy(output, m_compute.Execute(m_instanceData), sizeof(Planet::PlanetDescription) * m_instanceData.size());

    for (int i = 0; i < g_planets.size(); i++)
        g_planets[i].Update(timer, output[i]);
}

void PlanetRenderer::Render(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"Set vertex and index buffers");


    D3D12_VERTEX_BUFFER_VIEW vertexBuffers[] = {
        m_vertexBuffer.Flush(commandList),
        m_vertexBufferLow.Flush(commandList)
    };
    D3D12_VERTEX_BUFFER_VIEW instanceBuffers[] = {
        m_instanceBuffer.Flush(commandList)
    };
    D3D12_INDEX_BUFFER_VIEW indexBuffers[] = {
        m_indexBuffer.Flush(commandList),
        m_indexBufferLow.Flush(commandList)
    };

    commandList->IASetVertexBuffers(0, ARRAYSIZE(vertexBuffers), vertexBuffers);
    commandList->IASetVertexBuffers(1, ARRAYSIZE(instanceBuffers), instanceBuffers);
    commandList->IASetIndexBuffer(indexBuffers);

    PIXEndEvent(commandList);

    PIXBeginEvent(commandList, 0, L"Draw distant planets");

    if (g_planets.size() > 1)
    {
        m_distant.Execute(commandList);

        if (g_current == 0)
            commandList->DrawIndexedInstanced(m_graphicInfoLow.indices.size(), g_planets.size() - 1, 0, 0, 1);
        else if (g_current == g_planets.size() - 1)
            commandList->DrawIndexedInstanced(m_graphicInfoLow.indices.size(), g_planets.size() - 1, 0, 0, 0);
        else
        {
            commandList->DrawIndexedInstanced(m_graphicInfoLow.indices.size(), g_current, 0, 0, 0);
            commandList->DrawIndexedInstanced(m_graphicInfoLow.indices.size(), g_planets.size() - (g_current + 1), 0, 0, g_current + 1);
        }
    }

    PIXEndEvent(commandList);

    PIXBeginEvent(commandList, 0, L"Draw current planet");

    m_planet.Execute(commandList);
    commandList->DrawIndexedInstanced(m_graphicInfo.indices.size(), 1, 0, 0, g_current);

    m_atmosphere.Execute(commandList);
    commandList->DrawIndexedInstanced(m_graphicInfo.indices.size(), 1, 0, 0, g_current);

    PIXEndEvent(commandList);


}

const void PlanetRenderer::UpdateInstanceData()
{
    m_instanceData.clear();

    for (int i = 0; i < g_planets.size(); i++)
    {
        Planet& planet = g_planets[i];
        m_instanceData.push_back(planet.GetDescription());
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

        m_vertices.push_back(DirectX::VertexPositionNormalTexture(vertex, normal, tex));
    }
}

void PlanetRenderer::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    InputLayout inputLayout = InputLayout({
        { "SV_POSITION", 0, Vertex, Vector3::Zero },
        { "NORMAL", 0, Vertex, Vector3::Zero },
        { "TEXCOORD", 0, Vertex, Vector2::Zero },

        { "INST_ID", 1, Instance, 0 },
        { "INST_POSITION", 1, Instance, Vector3::Zero },
        { "INST_VELOCITY", 1, Instance, Vector3::Zero },
        { "INST_GRAVITY", 1, Instance, Vector3::Zero },
        { "INST_TIDAL", 1, Instance, Vector3::Zero },
        { "INST_RADIUS", 1, Instance, 0.f },
        { "INST_MASS", 1, Instance, 0.f },

        { "INST_COMPOSITION_SOIL_A", 1, Instance, 0.f },
        { "INST_COMPOSITION_SOIL_B", 1, Instance, 0.f },
        { "INST_COMPOSITION_ATMOSPHERE_A", 1, Instance, 0.f },
        { "INST_COMPOSITION_ATMOSPHERE_B", 1, Instance, 0.f },

        { "INST_MATERIAL_COLOR", 1, Instance, Vector4::Zero },
        { "INST_MATERIAL_KA", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_KD", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_KS", 1, Instance, Vector3::Zero },
        { "INST_MATERIAL_ALPHA", 1, Instance, 0.f }
    });

    //D3D12_SHADER_RESOURCE_VIEW_DESC texture{};
    //texture.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //texture.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    //texture.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //texture.Texture2D.MipLevels = 1;

    m_planet.SetInputLayout(inputLayout);
    m_planet.SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_planet.LoadShaders("SphereVertexShader", "SpherePixelShader");
    m_planet.SetConstantBuffers({
        g_mvp_buffer->Description,
        m_environment.Description
    });
    //m_pipeline.SetResource(texture, m_textureBuffer.Get());
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
    m_distant.LoadShaders("SphereVertexShader", "SpherePixelShader");
    m_distant.SetConstantBuffers({
        g_mvp_buffer->Description,
        m_environment.Description
        });
    m_distant.CreatePipeline();

    m_compute.LoadShader("ComputeShader");
    m_compute.CreatePipeline();

    // Get info for sphere mesh.
    UpdateVerticesInput(m_graphicInfo);
}