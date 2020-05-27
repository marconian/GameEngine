#pragma once

#include "pch.h"
#include "DeviceResources.h"
#include "Constants.h"
#include "Camera.h"
#include "Globals.h"

#include <map>
#include <vector>

using namespace DirectX::SimpleMath;

const std::unique_ptr<Camera> g_camera = std::make_unique<Camera>();

const Matrix g_world = Matrix::CreateWorld(Vector3::Zero, Vector3::Forward, Vector3::Up);

const std::unique_ptr<DX::DeviceResources> g_deviceResources = std::make_unique<DX::DeviceResources>(
    BACK_BUFFER_FORMAT,
    DEPTH_BUFFER_FORMAT,
    10,
    SAMPLE_COUNT,
    D3D_FEATURE_LEVEL_11_0,
    0,
    MSAA_ENABLED,
    SKY_COLOR
    );

std::vector<Planet> g_planets = std::vector<Planet>();
unsigned int g_current = 0;
std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>> g_mvp_buffer;

const void CreateGlobalBuffers()
{
    g_mvp_buffer = std::make_unique<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>();
}

const void UpdateGlobalBuffers()
{
    // Update ModelViewProjection buffer
    Matrix _mv = XMMatrixMultiply(g_world, g_camera->View());
    Matrix _mp = XMMatrixMultiply(g_world, g_camera->Proj());
    Matrix _vp = XMMatrixMultiply(g_camera->View(), g_camera->Proj());
    Matrix _mvp = XMMatrixMultiply(_mv, g_camera->Proj());

    Buffers::ModelViewProjection mvp = {};
    mvp.m = XMMatrixTranspose(g_world);
    mvp.v = XMMatrixTranspose(g_camera->View());
    mvp.p = XMMatrixTranspose(g_camera->Proj());
    mvp.mv = XMMatrixTranspose(_mv);
    mvp.mp = XMMatrixTranspose(_mp);
    mvp.vp = XMMatrixTranspose(_vp);
    mvp.mvp = XMMatrixTranspose(_mvp);
    mvp.eye = g_camera->Position();
    g_mvp_buffer->Write(mvp);
}

const unsigned int GetPlanetIndex(const int id)
{
    for (int i = 0; i < g_planets.size(); i++)
    {
        if (g_planets[i].id == id)
            return i;
    }

    return 0;
}

Planet& GetPlanet(unsigned int id)
{
    for (int i = 0; i < g_planets.size(); i++)
    {
        if (g_planets[i].id == id)
            return g_planets[i];
    }

    Planet planet{};
    return planet;
}

const void CleanPlanets() {
    const UINT32 id = g_planets[g_current].id;

    auto end = std::remove_if(g_planets.begin(), g_planets.end(),
        [](const Planet& planet) { 
            return planet.id == 0 || 
                isnan(planet.position.x + planet.position.y + planet.position.z) ||
                (Vector3::Distance(Vector3::Zero, planet.position) * S_NORM) > SUN_DIAMETER * 100; 
        });

    g_planets.erase(end, g_planets.end());
    //g_planets.shrink_to_fit();

    std::sort(g_planets.begin() + 1, g_planets.end(),
        [](const Planet& planet1, const Planet& planet2) {
            return planet1.mass < planet2.mass;
        });

    g_current = GetPlanetIndex(id);
}