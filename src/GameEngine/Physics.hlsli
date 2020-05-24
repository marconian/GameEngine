#pragma once

#include "Globals.hlsli"

float TerrainLevel(Instance instance, float3 position) {
    float _limit = (1.f / S_NORM) * 100;
    float _noise = scale(fractal(10,
        position.x + instance.id,
        position.y + instance.id,
        position.z + instance.id
    ), _limit);

    return instance.radius / S_NORM + _noise;
}

float3 TidalForce(float3 position, float3 radius, float3 tidal)
{
    float3 _tidal = tidal * radius * 2;
    float3 _pull = normalize(position) * abs(_tidal);

    //float3 _pull = normalize(input.position) * abs(normalize(pull));
    //float _max_pull = max(abs(_pull), 0);
    //if (_max_pull > 0)
    //    _position += _pull * .1f;

    return _pull;
}

double3 GravitationalAcceleration(Instance body1, Instance body2)
{
    double3 p_relative = (double3)body1.center - (double3)body2.center;
    double radius = distance(body1.center, body2.center);

    if (radius > 0) {
        double mass = (double)body1.mass;
        double mass_p = (double)body2.mass;
        double radius_m = radius * S_NORM * 1000;

        double g_force = (G * mass * mass_p) / pow(radius_m, 2);
        double acceleration = (g_force / mass) / S_NORM;

        double3 g_vector = normalize(-p_relative);
        g_vector *= acceleration;

        return g_vector;
    }

    return double3(0, 0, 0);
}



//// Calculate gravitational acceleration
//const Vector3 Planet::GetGravitationalAcceleration(Planet& planet)
//{
//    Vector3 p_relative = m_description.position - planet.GetPosition();
//
//    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
//    if (radius > 0)
//    {
//        const long double mass = m_description.mass;
//        const long double mass_p = planet.GetMass();
//        const long double radius_m = radius * S_NORM * 1000;
//
//        const long double g_force = (G * mass * mass_p) / pow(radius_m, 2);
//        const long double acceleration = (g_force / mass) / S_NORM;
//
//        Vector3 g_vector;
//        (-p_relative).Normalize(g_vector);
//
//        g_vector *= static_cast<float>(acceleration);
//
//        return g_vector;
//    }
//
//    return Vector3::Zero;
//}
//
//// Calculate gravitational acceleration
//const Vector3 Planet::GetTidalAcceleration(Planet& planet)
//{
//    Vector3 p_relative = m_description.position - planet.GetPosition();
//
//    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
//    if (radius > 0)
//    {
//        const long double mass = m_description.mass;
//        const long double radius_m = radius * S_NORM * 1000;
//
//        const long double t_force = G * (mass / pow(radius_m, 3));
//        const long double acceleration = t_force / S_NORM;
//
//        Vector3 t_vector;
//        (-p_relative).Normalize(t_vector);
//
//        t_vector *= static_cast<float>(acceleration);
//
//        return t_vector;
//    }
//
//    return Vector3::Zero;
//}