#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "DeviceResources.h"
#include "Planet.h"

#include <array>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Buffers;

using Microsoft::WRL::ComPtr;

Planet::Planet(const double mass, const Vector3 position, const Vector3 direction, const float velocity) :
    id(rand()),
    position(position),
    direction(Vector3::Zero),
    velocity(direction * velocity),
    angular(randv(0, 1) * rand(1e-3, 1e-6)),
    gravity(Vector3::Zero),
    tidal(Vector3::Zero),
    radius(pow(mass, 1 / 3.) / DENSITY_NORM),
    mass(mass),
    energy(.5 * mass * pow(velocity, 2)),
    material(),
    collisions(0),
    collision(false),
    quadrantMass(0)
{
    material.color = Vector4::One;
    material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
    material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
    material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    material.alpha = 0.f;

    if (mass > SUN_MASS * .5)
    {
        material.Ka = Vector3(1);
    }
}

const std::string Planet::GetQuadrant()
{
    const Vector3 p = position * S_NORM / g_quadrantSize;

    const std::string x = std::to_string((int)round(p.x));
    const std::string y = std::to_string((int)round(p.y));
    const std::string z = std::to_string((int)round(p.z));

    const std::string f = x + "|" + y + "|" + z;
    return f;
}

const void Composition::Normalize()
{
    float* values = (float*)this;
    size_t size = sizeof(Composition) / sizeof(float);

    float sum = 0;
    for (int i = 0; i < size; i++)
        sum += values[i];

    if (sum > 0)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] /= sum;
        }
    }
}

const void Composition::Randomize()
{
    std::array<float, 109> values = {};
    ZeroMemory(values.data(), sizeof(float) * values.size());

    for (int i = 0; i < 109; i++)
    {
        double a = ELEMENTAL_ABUNDANCE[i];
        values[i] = rand(0, a * 2.);
    }

    //size_t limit = (size_t)round(rand(5, 20));
    //
    //std::vector<UINT> idx{};
    //for (int i = 0; i < limit; i++)
    //    idx.push_back((UINT)round(rand(0, 108)));
    //
    //for (UINT i : idx)
    //    values[i] = rand(0, 1);

    memcpy((float*)this, values.data(), sizeof(Composition));

    Normalize();
}

const double Composition::GetAtmosphericMass(const Planet& planet)
{
    const double massPlanet = static_cast<double>(planet.mass);
    double mAtmosphere = 0; // atmospheric mass

    float* c = (float*)this;
    for (int i = 0; i < 109; i++)
    {
        if (ELEMENTAL_GOLDSCHMIDT[i] == 0)
            mAtmosphere += massPlanet * static_cast<double>(c[i]);
    }

    return mAtmosphere;
}

const void Composition::Degenerate(Planet& planet)
{
    const Planet star = g_planets[0];
    if (planet.id == star.id) return;

    const double radiusStar = static_cast<double>(star.radius);

    const double massPlanet = static_cast<double>(planet.mass);
    const double radiusPlanet = static_cast<double>(planet.radius);
    const double alpha = static_cast<double>(Vector3::Distance(star.position, planet.position)) * S_NORM;

    const double L = 4 * PI * pow(radiusStar, 2) * sigma * pow(5778., 4); // TEMP SUN in Kelvin = 5778.
    const double Ab = .306; // Bond albedo (https://en.wikipedia.org/wiki/Bond_albedo); Earth = .306
    const double vEscape = sqrt(2 * G * massPlanet / radiusPlanet);
    const double T = pow(L * (1 - Ab) / (16 * sigma * PI * pow(alpha, 2)), 1/4.); // Planetary equilibrium temperature

    double k = 0; // kinetic energy of particles

    float* c = (float*)this;
    for (int i = 0; i < 109; i++)
    {
        if (ELEMENTAL_GOLDSCHMIDT[i] == 0)
        {
            const double weight = ELEMENTAL_WEIGHT[i] / Na;
            const double vParticle = sqrt(3 * (kB * T / weight));

            const double mTotal = massPlanet * static_cast<double>(c[i]);

            k += .5 * mTotal * pow(vParticle, 2);
        }
    }

    const double v = sqrt(3 * k * T / massPlanet);

    if (v > vEscape)
    {
        float* c = (float*)this;

        float escapeRatio = sqrt((v - vEscape) / vEscape);
        float mSum = 0;

        for (int i = 0; i < 109; i++)
        {
            c[i] *= massPlanet;

            if (ELEMENTAL_GOLDSCHMIDT[i] == 0)
                c[i] *= escapeRatio;

            mSum += c[i];
        }

        Normalize();
    
        planet.mass = mSum;
        planet.radius = pow(planet.mass, 1 / 3.) / DENSITY_NORM;
    }
}

const Vector4 Composition::GetColor()
{
    float x = 0, y = 0, z = 0;

    float* c = (float*)this;
    for (int i = 0; i < 109; i++)
    {
        Vector4 atom = (Vector4)ATOM_COLORS[i];
        float weight = (float)(1 - ELEMENTAL_WEIGHT_N[i]);
        float goldschmidt = ELEMENTAL_GOLDSCHMIDT[i];

        if (goldschmidt == 1 || goldschmidt == 3)
        {
            x += atom.x * c[i];
            y += atom.y * c[i];
            z += atom.z * c[i];
        }
    }
    
    Vector3 color = Vector3(x, y, z);
    color.Normalize();

    return Vector4(color.x, color.y, color.z, 1);
}