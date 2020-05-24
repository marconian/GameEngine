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

Planet::Planet(const double mass, const double size, const XMVECTORF32 color) :
    m_origin(Vector3::Zero),
    m_description() 
{
    m_description.id = (unsigned int)rand(1, 1000);
    m_description.mass = mass;
    m_description.radius = size / 2.;

    m_description.material.color = color;
    m_description.material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
    m_description.material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
    m_description.material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    m_description.material.alpha = 0.f;

    m_description.atmosphere.water = .4f;
    m_description.atmosphere.soil = .5f;

    m_description.soil.water = .5f;
    m_description.soil.soil = .5f;

    if (m_description.mass > (SUN_MASS / M_NORM) * .5)
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

// Calculate gravitational acceleration
const Vector3 Planet::GetGravitationalAcceleration(Planet& planet)
{
    Vector3 p_relative = m_description.position - planet.GetPosition();

    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
    if (radius > 0)
    {
        const long double mass = m_description.mass * M_NORM;
        const long double mass_p = planet.GetMass() * M_NORM;
        const long double radius_m = radius * S_NORM * 1000;

        const long double g_force = (G * mass * mass_p) / pow(radius_m, 2);
        const long double acceleration = (g_force / mass) / S_NORM;

        Vector3 g_vector;
        (-p_relative).Normalize(g_vector);

        g_vector *= static_cast<float>(acceleration);

        return g_vector;
    }

    return Vector3::Zero;
}

// Calculate gravitational acceleration
const Vector3 Planet::GetTidalAcceleration(Planet& planet)
{
    Vector3 p_relative = m_description.position - planet.GetPosition();

    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
    if (radius > 0)
    {
        const long double mass = m_description.mass * M_NORM;
        const long double radius_m = radius * S_NORM * 1000;

        const long double t_force = G * (mass / pow(radius_m, 3));
        const long double acceleration = t_force / S_NORM;

        Vector3 t_vector;
        (-p_relative).Normalize(t_vector);

        t_vector *= static_cast<float>(acceleration);

        return t_vector;
    }

    return Vector3::Zero;
}