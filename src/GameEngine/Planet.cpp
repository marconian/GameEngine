#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "Planet.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

Planet::Planet(const double mass, const double radius, const XMVECTORF32 color) :
    m_origin(Vector3::Zero),
    m_description() 
{
    m_description.id = rand();
    m_description.mass = mass;
    m_description.radius = radius;

    m_description.material.color = color;
    m_description.material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
    m_description.material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
    m_description.material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    m_description.material.alpha = 0.f;

    m_description.atmosphere.water = .4f;
    m_description.atmosphere.soil = .5f;

    m_description.soil.water = .5f;
    m_description.soil.soil = .5f;

    if (m_description.mass > SUN_MASS * .5)
    {
        m_description.material.Ka = Vector3(1);
        m_description.atmosphere.water = 0.f;
    }
}

void Planet::Update(DX::StepTimer const& timer, Planet::PlanetDescription description)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    const long double radius = Vector3::Distance(Vector3::Zero, m_description.position);

    m_description = description;

    if (radius < EARTH_SUN_DIST * 3)
    {
        //m_description.gravity = Vector3::Zero;
        //m_description.tidal = Vector3::Zero;
        //if (m_description.position != Vector3::Zero && g_planets.size() > 1) {
        //   /* #pragma omp parallel for
        //    for (int i = 0; i < g_planets.size(); i++)
        //    {
        //        Planet& p = g_planets[i];
        //        m_description.gravity += GetGravitationalAcceleration(p);
        //        m_description.tidal += GetTidalAcceleration(p);
        //    }*/

        //    m_description.gravity = gravity;
        //}

        //m_description.velocity += m_description.gravity;
        m_description.position += Vector3::Lerp(Vector3::Zero, m_description.velocity, TIME_DELTA * elapsedTime);
    }
    else
    {
        //g_planets.erase(m_id);
    }
}