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
    radius(RadiusByMass(mass)),
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

const float Planet::RadiusByMass(double mass)
{
    double r = sqrt(mass) * MASS_RADIUS_NORM + MASS_RADIUS_OFFSET;
    r -= ((pow(mass, 2) * G) / mass) * 1.861399E-11;

    return static_cast<float>(r);
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
    const size_t size = sizeof(Composition) / sizeof(float);
    std::array<double, size> values = {};
    for (int i = 0; i < size; i++)
        values[i] = static_cast<double>(((float*)this)[i]);

    double sum = 0;
    for (int i = 0; i < size; i++)
        sum += values[i];

    if (sum > 0)
    {
        for (int i = 0; i < size; i++)
            values[i] /= sum;
    }

    for (int i = 0; i < size; i++)
        ((float*)this)[i] = static_cast<float>(values[i]);
}

const void Composition::Randomize(const Planet& planet)
{
    const size_t size = sizeof(Composition) / sizeof(float);
    std::array<double, size> values = {};
    ZeroMemory(values.data(), sizeof(float) * values.size());

    const double* rVector = planet.mass > 1e25 ? ELEMENTAL_ABUNDANCE : ELEMENTAL_ABUNDANCE_T;

    for (int i = 0; i < 109; i++)
    {
        double a = ELEMENTAL_ABUNDANCE[i];
        values[i] = static_cast<double>(rand(0, a * 2.));
    }

    for (int i = 0; i < size; i++)
        ((float*)this)[i] = static_cast<float>(values[i]);

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

const double Composition::Degenerate(const Planet& planet)
{
    const size_t size = sizeof(Composition) / sizeof(float);
    std::array<double, size> values = {};
    for (int i = 0; i < size; i++)
        values[i] = static_cast<double>(((float*)this)[i]);

    const Planet star = g_planets[0];
    if (planet.id == star.id) return planet.mass;

    const double radiusStar = static_cast<double>(star.radius);

    const double massPlanet = static_cast<double>(planet.mass);
    const double radiusPlanet = static_cast<double>(planet.radius);
    const double volumePlanet = pow(radiusPlanet, 3) * PI * (3 / 4.);
    const double alpha = static_cast<double>(Vector3::Distance(star.position, planet.position)) * S_NORM;

    const double L = 4 * PI * pow(radiusStar, 2) * sigma * pow(5778., 4); // TEMP SUN in Kelvin = 5778.
    const double Ab = .306; // Bond albedo (https://en.wikipedia.org/wiki/Bond_albedo); Earth = .306
    const double vEscape = sqrt(2 * G * massPlanet / radiusPlanet);
    const double T = pow(L * (1 - Ab) / (16 * sigma * PI * pow(alpha, 2)), 1/4.); // Planetary equilibrium temperature

    double mAtmosphere = 0;
    for (int i = 0; i < 109; i++)
    {
        values[i] *= massPlanet; // set all values to actual mass
        if (ELEMENTAL_GOLDSCHMIDT[i] == 0) 
            mAtmosphere += values[i];
    }


    const double U = -((6 / 5.) * G * pow(massPlanet, 2) / radiusPlanet);
    const double F = -U;

    const double Rp = Planet::RadiusByMass(massPlanet - mAtmosphere);
    const double Ra = radiusPlanet - Rp;

    const double Va = volumePlanet - pow(Rp, 3) * PI * (3 / 4.);

    const double Aa = 4 * PI * pow(Rp, 2); // m2
    const double Pa = mAtmosphere / Aa;// F / surfaceArea;
    const double Ka = Pa * Va / (2 / 3.);
    //const double N = mAtmosphere * 1000 / ELEMENTAL_WEIGHT[5] * Da;
    const double Ta = (2 / 3.) * (Ka / mAtmosphere);

    const double oxygen = values[7] * (1. / ((ELEMENTAL_WEIGHT[7] / Na) * 1000.)) * .5;

    double totalMass = 0;
    for (int i = 0; i < 109; i++)
    {
        std::string name = ELEMENTAL_SYMBOLS[i];

        if (ELEMENTAL_GOLDSCHMIDT[i] == 0)
        {
            const double mParticle = ELEMENTAL_WEIGHT[i] / Na;
            const double uParticle = sqrt(2 * kB * Ta / mParticle);
            const double vParticle = sqrt(3 * (kB * Ta / mParticle));

            double boundMass = 0;
            if (OXYGEN_BOND[i])
                boundMass = mParticle * (oxygen * (values[i] / mAtmosphere));

            const double k = .5 * (values[i] - boundMass) * pow(vParticle, 2);
            const double v = sqrt(3 * k * T / massPlanet);

            if (vParticle > vEscape)
            {
                float escapeRatio = pow((v - vEscape) / vEscape, 2);
                values[i] *= 1 - (escapeRatio > .01 ? .01 : escapeRatio);
            }
        }

        totalMass += values[i];
    }

    for (int i = 0; i < size; i++)
        ((float*)this)[i] = static_cast<float>(values[i]);

    Normalize();

    return totalMass;
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

    float vSum = x + y + z;
    
    Vector3 color = Vector3(x, y, z);
    color.Normalize();

    return Vector4(color.x, color.y, color.z, 1);
}