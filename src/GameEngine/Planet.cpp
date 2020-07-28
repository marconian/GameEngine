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

template struct Composition<float>;
template struct Composition<double>;

Planet::Planet(const double mass, double density, double temperature, const Vector3 position, const Vector3 direction, const float velocity) :
    id(rand() * rand()),
    position(position),
    direction(Vector3::Zero),
    velocity(direction * velocity),
    angular(randv(0, 1) * rand(1e-3, 1e-6)),
    radius(RadiusByMass(mass)),
    mass(mass),
    temperature(temperature),
    density(density),
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

std::vector<DepthInfo>& Planet::GetDensityProfile()
{
    if (g_profiles.find(id) == g_profiles.end())
    {
        // Current object Properties
        auto tComposition = g_compositions[id].As<double>();

        double const coreDensity = static_cast<double>(density);

        size_t const step = static_cast<size_t>(round(pow(mass * 1e-9, .35)));
        size_t const size = sizeof(Composition<float>) / sizeof(float);
        double usedMass = 0, usedVolume = 0;

        if (step == 0 || g_compositions.find(id) == g_compositions.end())
            return std::vector<DepthInfo>();

        std::vector<ElementInfo> store{};
        for (uint32_t i = 0; i < size; i++)
            store.push_back({ i + 1, ELEMENTAL_WEIGHT[i], ELEMENTAL_DENSITY[i] * 1000., tComposition.data()[i] });

        std::sort(store.begin(), store.end(),
            [](const ElementInfo& a, const ElementInfo& b) {
                return a.weight < b.weight;
            });

        std::vector<DepthInfo> profile{}; int i = 1;
        double d = density, dp = 0, p = 0;
        while (store.size() > 0)
        {
            DepthInfo info{};
            info.radius = step * i;
            info.area = PI_SQ * pow(info.radius, 2); // m2
            info.volume = PI_CB * pow(info.radius, 3) - usedVolume;
            info.density = d;
            info.mass = info.volume * info.density;

            usedVolume += info.volume;
            usedMass += info.mass;

            info.pressure = (usedMass * G) / pow(info.radius, 2);
            p = info.pressure - p;

            std::array<double, size> composition{};

            double m = info.mass, ma = 0;
            for (int j = store.size() - 1; j >= 0; j--)
            {
                if (store[j].mass > 0)
                {
                    double use = m > store[j].mass ? store[j].mass : m;

                    use *= rand(.5, 1);

                    ma += (use / info.density) * store[j].density;
                    d += store[j].density * use;
                    store[j].mass -= use;
                    m -= use;

                    composition[j] = use;
                }

                if (store[j].mass == 0) store.erase(store.end() - 1);
                if (m == 0) break;
            }

            d /= info.mass;
            info.composition = composition;

            profile.push_back(info); i++;

            if (d < .01) break;
        }

        mass = usedMass;
        g_compositions[id] /= g_compositions[id].sum();
        g_compositions[id] *= mass;

        g_profiles[id] = profile;

        RefreshDensityProfile();

        //double P = profile[0].pressure, T = temperature;
        //for (DepthInfo& info : profile)
        //{
        //    double pressureStep = P - info.pressure;
        //    T += -(info.pressure / T) * pressureStep;
        //
        //    info.temperature = T;
        //
        //    P = info.pressure;
        //}
    }

    return g_profiles[id];
}

void Planet::RefreshDensityProfile()
{
    std::vector<DepthInfo>& profile = g_profiles[id];

    double usedVolume = 0, usedMass = 0;
    for (int i = 0; i < profile.size(); i++)
    {
        DepthInfo& info = profile[i];

        if (i == profile.size() - 1)
        {
            info.volume = info.mass / info.density;
            info.radius = cbrt(((info.volume + usedVolume) / PI) * (3 / 4.));
        }
        else
        {
            info.density = info.mass / info.volume;
        }

        usedMass += info.mass;
        info.pressure = (usedMass * G) / pow(info.radius, 2);

        usedVolume += info.volume;
    }
}

std::optional<double> Planet::RadiusByDensity()
{
    std::vector<DepthInfo> const& profile = GetDensityProfile();

    if (profile.size() > 0)
    {
        double radius = profile[profile.size()-1].radius;
        return radius;
    }

    return std::nullopt;
}

std::optional<double> Planet::MassByDensity()
{
    std::vector<DepthInfo> const& profile = GetDensityProfile();

    double m = 0;
    for (DepthInfo const& info : profile)
        m += info.mass;

    if (m > 0) return m;
    else return std::nullopt;
}

const void Planet::Update(float const deltaTime)
{
    auto m = MassByDensity(), r = RadiusByDensity();

    if (!m.has_value() || !r.has_value() || g_compositions.find(id) == g_compositions.end())
        return;

    // Stellar properties
    Planet const star = g_planets[0];
    double const sRadius = star.GetRadius();
    double const sVolume = pow(sRadius, 3) * PI_CB;
    double const sLuminosity = PI_SQ * pow(sRadius, 2) * sigma * pow(5778., 4); // TEMP SUN in Kelvin = 5778.

    // Current object Properties
    double const tMass = m.value();
    double const tRadius = r.value();
    double const tVolume = pow(tRadius, 3) * PI_CB;
    double const tDistance = static_cast<double>(Vector3::Distance(star.position, position)) * S_NORM; // Distance to star (alpha)
    std::vector<DepthInfo>& tProfile = g_profiles[id];
    std::vector<DepthInfo> refProfile = g_profiles[id];

    //const double alpha = sqrt(pow(star.position.x - planet.position.x, 2) + pow(star.position.y - planet.position.y, 2) + pow(star.position.z - planet.position.z, 2));
    const double Ab = static_cast<double>(material.color.x + material.color.y + material.color.z) / 3.; // Bond albedo (https://en.wikipedia.org/wiki/Bond_albedo); Earth = .306
    //const double T = pow(sLuminosity * (1 - Ab) / (16 * sigma * PI * pow(alpha, 2)), 1 / 4.); // Planetary equilibrium temperature

    bool lostToSpace = false; double insideMass = 0, outsideMass = tMass;
    for (int j = 0; j < tProfile.size(); j++)
    {
        DepthInfo& layer = tProfile[j];
        DepthInfo const& layerRef = refProfile[j];

        insideMass += layerRef.mass;
        outsideMass = tMass - insideMass;

        double const pEscape = sqrt((2 * G * insideMass) / layerRef.radius);
        double const aK = (layerRef.pressure * layerRef.volume) / (2 / 3.);

        for (int i = 0; i < layer.composition.size(); i++)
        {
            std::string const name = ELEMENTAL_SYMBOLS[i];
            double const layerParticleMass = layerRef.composition.data()[i];
            if (layerParticleMass > 0) //&& ELEMENTAL_GOLDSCHMIDT[i] == 0
            {
                double const nParticles = layerParticleMass / (ELEMENTAL_WEIGHT[i] / Na);
                double const nT = (layerRef.pressure * layerRef.volume) / R;
                double const tParticle = (layerRef.pressure * layerRef.volume) / (nParticles * R);

                double vParticle = sqrt(3 * (kB * tParticle / layerParticleMass));
                vParticle = rand(vParticle * .5, vParticle * 1.5);

                //const double k = .5 * (values[i]) * pow(vParticle, 2); // - boundMass
                //const double v = sqrt(3 * k * T / massPlanet);

                double escapeRatio = sqrt(abs(vParticle - pEscape) / max(vParticle, pEscape)) * (deltaTime / 3600.);
                escapeRatio = escapeRatio < 1 ? escapeRatio : 1;
                double change = layerParticleMass * escapeRatio;

                if (vParticle > pEscape)
                {
                    // inner layers
                    if (j < tProfile.size() - 1)
                    {
                        DepthInfo& layerUp = tProfile[j + 1];
                        layerUp.composition.data()[i] += change;
                    }
                    // most outer layer
                    else lostToSpace = true;

                    layer.composition.data()[i] -= change;
                }
                else
                {
                    // outer layers
                    if (j > 0)
                    {
                        DepthInfo& layerDown = tProfile[j - 1];
                        layerDown.composition.data()[i] += change;

                        layer.composition.data()[i] -= change;
                    }
                }
            }
        }
    }

    for (int j = tProfile.size() - 1; j >= 0; j--)
    {
        DepthInfo& layer = tProfile[j];

        for (int i = 0; i < layer.composition.size(); i++)
        {
            if (layer.composition.data()[i] < 0 || isnan(layer.composition.data()[i]))
                layer.composition.data()[i] = 0;
        }
        layer.mass = layer.composition.sum();

        if (isnan(layer.mass))
            int a = 0;

        if (layer.mass == 0)
        {
            if (j > 0) tProfile.erase(tProfile.begin() + j);
        }
    }

    // Write values back only on change
    if (lostToSpace && tProfile.size() > 0)
    {
        Composition<double> values = {};
        for (int j = 0; j < tProfile.size(); j++)
            values += tProfile[j].composition;

        m = MassByDensity();
        mass = m.has_value() ? m.value() : 0;

        g_compositions[id] = values.As<float>();
    }

    RefreshDensityProfile();

    r = RadiusByDensity();
    radius = r.has_value() ? r.value() : 1;

    if (tProfile.size() > 0)
        material.color = tProfile[tProfile.size() - 1].composition.GetColor();

    //const double sLuminosity = PI_SQ * pow(planet.radius, 2) * sigma * pow(5778., 4); // TEMP SUN in Kelvin = 5778.
    if (mass > SUN_MASS * .4)
        material.Ka = Vector3(1);
    //else material.Ka = Vector3(.2);
}

template<typename T>
void Composition<T>::Randomize(const Planet& planet)
{
    size_t const s = sizeof(Composition<T>) / sizeof(T);
    std::array<T, s> values = {};
    ZeroMemory(values.data(), sizeof(T) * values.size());

    const double* rVector = planet.mass > EARTH_MASS * 10 ? ELEMENTAL_ABUNDANCE : ELEMENTAL_ABUNDANCE_T;

    for (int i = 0; i < s; i++)
    {
        double a = rVector[i];
        values[i] = rand(0, a * 2.);
    }

    normalize(values);

    *this = values;
    *this *= planet.mass;
}

template<typename T>
Vector4 Composition<T>::GetColor() const
{
    double x = 0., y = 0., z = 0.;

    Composition<double> values = As<double>();
    values /= values.sum();

    for (int i = 0; i < size(); i++)
    {
        Vector4 atom = (Vector4)ELEMENTAL_COLORS[i];
        UINT goldschmidt = ELEMENTAL_GOLDSCHMIDT[i];

        const double strength = values.data()[i];

        x += static_cast<double>(atom.x) * strength;
        y += static_cast<double>(atom.y) * strength;
        z += static_cast<double>(atom.z) * strength;
    }

    if (isinf(x) || isinf(y) || isinf(z))
        return Vector4::One;

    long double vSum = sqrt(x * x + y * y + z * z);
    while (isinf(vSum))
    {
        x /= 1.e10, y /= 1.e10, z /= 1.e10;
        vSum = sqrt(x * x + y * y + z * z);
    }
    if (vSum > 0) vSum = 1. / vSum;

    x *= vSum;
    y *= vSum;
    z *= vSum;
    
    Vector4 color = Vector4(x, y, z, 1);
    return color;
}