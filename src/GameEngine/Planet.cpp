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

Planet::Planet(const double mass, const Vector3 position, const Vector3 direction, const float velocity) :
    id(rand()),
    position(position),
    direction(Vector3::Zero),
    velocity(direction * velocity),
    angular(randv(0, 1) * rand(1e-3, 1e-6)),
    radius(RadiusByMass(mass)),
    mass(mass),
    temperature(),
    density(sqrt(sqrt(mass) / (mass > 5e25 ? rand(6e5, 7e5) : rand(1e5, 2e5)))),
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

std::vector<DepthInfo> Planet::GetDensityProfile() const
{
    // Current object Properties
    double const tMass = static_cast<double>(mass);
    double const coreDensity = static_cast<double>(density);
    if (g_compositions.find(id) != g_compositions.end())
    {
        size_t const step = static_cast<size_t>(round(pow(tMass * 1e-9, .35)));
        size_t const size = sizeof(Composition) / sizeof(float);
        double usedMass = 0, usedVolume = 0;

        if (step == 0) return {};

        std::vector<double> store{};
        store.resize(size);
        for (int i = 0; i < size; i++)
            store[i] = static_cast<double>(((float*)&g_compositions[id])[i]) * tMass;

        std::vector<DepthInfo> profile{}; int i = 1; double d = density * .1;
        while (usedMass < tMass)
        {
            DepthInfo info{};
            info.radius = i * step;
            info.density = d;
            info.volume = PI_CB * pow(info.radius, 3) - usedVolume;
            info.mass = info.volume * info.density;
            
            std::array<double, size> composition{};

            usedVolume += info.volume;
            usedMass += info.mass;

            double m = info.mass;

            d = 0;
            for (int j = store.size() - 1; j >= 0; j--)
            {
                if (store[j] > 0)
                {
                    double use = m > store[j] ? store[j] : m;
                    double v = use / info.density;

                    d += (v * ELEMENTAL_WEIGHT[j]) / v;
                    store[j] -= use;
                    m -= use;

                    composition[j] = use;
                }

                if (store[j] == 0) store.erase(store.end() - 1);
                if (m == 0) break;
            }

            info.composition = {};
            normalize(composition);
            for (int j = 0; j < size; j++)
                ((float*)&info.composition)[j] = static_cast<float>(composition[j]);

            profile.push_back(info); i++;
        }

        double mSum = 0, dMax = 0;
        for (DepthInfo& info : profile)
        {
            mSum += info.mass;
            dMax = info.density > dMax ? info.density : dMax;
        }

        usedVolume = 0, usedMass = 0;
        for (DepthInfo& info : profile)
        {
            info.radius *= .1;
            info.area = PI_SQ * pow(info.radius, 2); // m2
            info.volume = (PI_CB * pow(info.radius, 3)) - usedVolume;
            info.mass = (info.mass / mSum) * mass;
            info.density = info.mass / info.volume;
            info.pressure = (info.mass * G) / info.area;

            for (int j = 0; j < size; j++)
                ((float*)&info.composition)[j] *= static_cast<float>(info.mass);

            usedVolume += info.volume;
        }

        return profile;
    }

    return {};
}

std::optional<float> Planet::RadiusByDensity() const
{
    std::vector<DepthInfo> const& profile = GetDensityProfile();
    g_profiles[id] = profile;

    if (profile.size() > 1)
    {
        float radius = profile[profile.size()-1].radius;

        //double tm = 0, td = 0, tv = 0;
        //for (auto const& info : profile)
        //{
        //    tm += info.mass;
        //    td += info.density;
        //    tv += info.volume;
        //}
        //
        //td /= static_cast<double>(profile.size());

        return radius;
    }

    return std::nullopt;
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
        values[i] = rand(0, a * 2.);
    }

    normalize(values);

    for (int i = 0; i < size; i++)
        ((float*)this)[i] = static_cast<float>(values[i]);
}

const double Composition::GetPlanetMass(const Planet& planet)
{
    const double tMass = static_cast<double>(planet.mass);
    double pMass = 0; // atmospheric mass

    float* c = (float*)this;
    for (int i = 0; i < 109; i++)
    {
        if (ELEMENTAL_GOLDSCHMIDT[i] != 0)
            pMass += tMass * static_cast<double>(c[i]);
    }

    return pMass;
}

const double Composition::Degenerate(Planet const& planet, DX::StepTimer const& timer)
{
    const float deltaTime = g_speed * float(timer.GetElapsedSeconds());

    // Stellar properties
    const Planet star = g_planets[0];
    const double sRadius = star.GetRadius();
    const double sVolume = pow(sRadius, 3) * PI_CB;
    const double sLuminosity = PI_SQ * pow(sRadius, 2) * sigma * pow(5778., 4); // TEMP SUN in Kelvin = 5778.

    // Current object Properties
    const double tMass = static_cast<double>(planet.mass);
    const double tRadius = static_cast<double>(planet.radius);
    const double tVolume = pow(tRadius, 3) * PI_CB;
    const double tDistance = static_cast<double>(Vector3::Distance(star.position, planet.position)) * S_NORM; // Distance to star (alpha)
    std::vector<DepthInfo>& tProfile = g_profiles[planet.id];

    // Current object composition
    const size_t size = sizeof(Composition) / sizeof(float);
    std::array<double, size> values = {};
    for (int i = 0; i < size; i++)
        values[i] = static_cast<double>(((float*)this)[i]) * tMass;

    const double Ab = .306; // Bond albedo (https://en.wikipedia.org/wiki/Bond_albedo); Earth = .306
    //const double T = pow(L * (1 - Ab) / (16 * sigma * PI * pow(alpha, 2)), 1 / 4.); // Planetary equilibrium temperature

    //const double pMass = GetPlanetMass(planet);
    //const double pRadius = Planet::RadiusByMass(pMass);
    //const double pVolume = PI_CB * pow(pRadius, 3);
    //const double pEscape = sqrt(2 * G * tMass / tRadius);

    //const double aMass = tMass - pMass;
    //const double aRadius = tRadius - pRadius;
    //const double aVolume = tVolume - pVolume;

    //const double pArea = PI_SQ * pow(pRadius, 2); // m2
    //const double aPressure = (pMass * G) / pArea;
    //const double aDensity = aMass / aVolume;
    //const double aK = layer.pressure * layer.volume / (2 / 3.);
    //const double Ta = (2 / 3.) * (Ka / Ma);

    //const double oxygen = values[7] * (1. / ((ELEMENTAL_WEIGHT[7] / Na) * 1000.)) * .5;

    bool lostToSpace = false; double insideMass = 0, outsideMass = planet.mass;
    for (int j = 0; j < tProfile.size(); j++)
    {
        DepthInfo& layer = tProfile[j];
        insideMass += layer.mass;
        outsideMass -= layer.mass;

        const double pEscape = sqrt((2 * G * insideMass) / layer.radius);
        const double aK = (layer.pressure * layer.volume) / (2 / 3.);

        for (int i = 0; i < size; i++)
        {
            std::string const name = ELEMENTAL_SYMBOLS[i];
            double layerParticleMass = static_cast<double>(((float*)&layer.composition)[i]);
            if (layerParticleMass > 0 && ELEMENTAL_GOLDSCHMIDT[i] == 0)
            {
                //const double mParticle = (ELEMENTAL_WEIGHT[i] / Na);
                const double mParticle = layerParticleMass;
                //const double tParticle = (mParticle * .5 * layer.pressure) / (layer.volume * aK);
                const double tParticle = (layer.pressure * layer.volume) / (mParticle * R);
                const double vParticle = sqrt(3 * (kB * tParticle / mParticle));

                //const double k = .5 * (values[i]) * pow(vParticle, 2); // - boundMass
                //const double v = sqrt(3 * k * T / massPlanet);

                if (vParticle > pEscape)
                {
                    double escapeRatio = sqrt((vParticle - pEscape) / vParticle) * deltaTime;
                    escapeRatio = escapeRatio < 1 ? escapeRatio : 1;
                
                    double change = values[i] * escapeRatio;
                
                    // inner layers
                    if (j < tProfile.size() - 1)
                    {
                        DepthInfo& layerUp = tProfile[j + 1];
                        layerUp.mass += change;
                        ((float*)&layerUp.composition)[i] += change;
                    }
                    // most outer layer
                    else
                    {
                        values[i] -= values[i] * escapeRatio;
                        lostToSpace = true;
                    }
                
                    layer.mass -= change;
                    ((float*)&layer.composition)[i] -= change;
                }
            }
        }
    }


    double totalMass = 0;
    for (int i = 0; i < size; i++)
        totalMass += values[i];

    // Write values back only on change
    if (lostToSpace)
    {
        normalize(values);
        for (int i = 0; i < size; i++)
            ((float*)this)[i] = static_cast<float>(values[i]);
    }

    return totalMass;
}

const Vector4 Composition::GetColor()
{
    float x = 0, y = 0, z = 0;

    float* c = (float*)this;
    for (int i = 0; i < 109; i++)
    {
        Vector4 atom = (Vector4)ATOM_COLORS[i];
        float weight = static_cast<float>(1 - ELEMENTAL_WEIGHT_N[i]);
        UINT goldschmidt = ELEMENTAL_GOLDSCHMIDT[i];

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