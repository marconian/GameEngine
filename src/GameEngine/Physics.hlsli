#pragma once

#include "Constants.hlsli"
#include "SimplexNoise.hlsli"

float TerrainLevel(Instance instance, float3 position) {
    float _limit = (1.f / S_NORM) * 100;
    float _noise = scale(fractal(10,
        position.x + instance.id,
        position.y + instance.id,
        position.z + instance.id
    ), _limit);

    return instance.radius + _noise;
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
        double mass = (double)body1.mass * M_NORM;
        double mass_p = (double)body2.mass * M_NORM;
        double radius_m = radius * S_NORM * 1000;

        double g_force = (G * mass * mass_p) / pow(radius_m, 2);
        double acceleration = (g_force / mass) / S_NORM;

        double3 g_vector = normalize(-p_relative);
        g_vector *= acceleration;

        return g_vector;
    }

    return double3(0, 0, 0);
}