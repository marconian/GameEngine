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
    2,
    SAMPLE_COUNT,
    D3D_FEATURE_LEVEL_11_0,
    0,
    MSAA_ENABLED,
    SKY_COLOR
    );

unsigned int g_current = 0;
unsigned int g_quadrantSize = QUADRANT_SIZE;
unsigned int g_collisions = 0;
float g_speed = TIME_DELTA;
bool g_coreView = false;

std::vector<Planet> g_planets = {};
std::map<uint32_t, Composition<float>> g_compositions = {};
std::map<uint32_t, std::vector<DepthInfo>> g_profiles = {};
std::map<std::string, std::vector<Planet*>> g_quadrants = {};


std::unique_ptr<Buffers::ConstantBuffer<Buffers::Settings>> g_settings_buffer;
std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>> g_mvp_buffer;

const void CreateGlobalBuffers()
{
    g_settings_buffer = std::make_unique<Buffers::ConstantBuffer<Buffers::Settings>>();
    g_mvp_buffer = std::make_unique<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>();
}

const void UpdateGlobalBuffers()
{
    Buffers::Settings settings = {};
    settings.coreView = g_coreView;
    g_settings_buffer->Write(&settings);

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
    g_mvp_buffer->Write(&mvp);
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

    return Planet{};
}

const unsigned int CleanPlanets() 
{
    const UINT32 id = g_planets[g_current].id;
    const double maxDistance = PLUTO_SUN_DIST * S_NORM_INV;

    for (int i = g_planets.size() - 1; i >= 0; i--)
    {
        const Planet& planet = g_planets[i];
        bool erase = planet.mass == 0 || // ID is zero when destroyed
            isnan(planet.position.x + planet.position.y + planet.position.z) || // Error value
            sqrt(pow(planet.position.x, 2) + pow(planet.position.y, 2) + pow(planet.position.z, 2)) >= maxDistance;
        
        if (erase) g_planets.erase(g_planets.begin() + i);
    }

    //auto end = std::remove_if(g_planets.begin(), g_planets.end(),
    //    [maxDistance](Planet const& planet) {
    //        return planet.mass == 0 || // ID is zero when destroyed
    //            isnan(planet.position.x + planet.position.y + planet.position.z) || // Error value
    //            sqrt(pow(planet.position.x, 2) + pow(planet.position.y, 2) + pow(planet.position.z, 2)) >= maxDistance;
    //    });
    //
    //g_planets.erase(end, g_planets.end());

    std::sort(g_planets.begin(), g_planets.end(),
        [](const Planet& planet1, const Planet& planet2) {
            return planet1.mass > planet2.mass;
        });

    const UINT32 idx = GetPlanetIndex(id);
    return idx;
}