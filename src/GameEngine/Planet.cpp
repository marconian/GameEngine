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
    m_id(rand()),
    m_mass(mass),
    m_size(size),
    m_position(Vector3::Zero),
    m_origin(Vector3::Zero),
    m_velocity(Vector3::Zero),
    m_rotation(Quaternion::Identity),
    m_color(color) { }

void Planet::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    const long double radius = Vector3::Distance(Vector3::Zero, m_position);

    if (radius < EARTH_SUN_DIST * 3)
    {
        m_gravity = Vector3::Zero;
        m_tidal = Vector3::Zero;
        if (m_position != Vector3::Zero && g_planets.size() > 1) {
            #pragma omp parallel for
            for (int i = 0; i < g_planets.size(); i++)
            {
                Planet& p = g_planets[i];
                m_gravity += GetGravitationalAcceleration(p);
                m_tidal += GetTidalAcceleration(p);
            }
        }

        m_velocity += m_gravity;
        m_position += Vector3::Lerp(Vector3::Zero, m_velocity, TIME_DELTA * elapsedTime);
    }
    else
    {
        //g_planets.erase(m_id);
    }
}

// Calculate gravitational acceleration
const Vector3 Planet::GetGravitationalAcceleration(Planet& planet)
{
    Vector3 p_relative = m_position - planet.GetPosition();

    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
    if (radius > 0)
    {
        const long double mass = m_mass * M_NORM;
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
    Vector3 p_relative = m_position - planet.GetPosition();

    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
    if (radius > 0)
    {
        const long double mass = m_mass * M_NORM;
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