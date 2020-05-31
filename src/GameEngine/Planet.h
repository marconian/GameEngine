#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

struct Planet
{
public:
    Planet() {}
    Planet(const double mass, const double radius, const DirectX::SimpleMath::Vector3 position, const DirectX::SimpleMath::Vector3 direction, const float velocity, const DirectX::XMVECTORF32 color = DirectX::Colors::White) noexcept(false);
    Planet(const Planet& planet) :
        id(planet.id),
        position(planet.position),
        direction(planet.direction),
        velocity(planet.velocity),
        angular(planet.angular),
        gravity(planet.gravity),
        tidal(planet.tidal),
        radius(planet.radius),
        mass(planet.mass),
        soil(planet.soil),
        atmosphere(planet.atmosphere),
        material(planet.material),
        collisions(planet.collisions) { }
    //Planet& operator=(const Planet& planet) = delete;

    unsigned int                    id;
    DirectX::SimpleMath::Vector3    position;
    DirectX::SimpleMath::Vector3    direction;
    DirectX::SimpleMath::Vector3    velocity;
    DirectX::SimpleMath::Vector3    angular;
    DirectX::SimpleMath::Vector3    gravity;
    DirectX::SimpleMath::Vector3    tidal;
    float                           radius;
    float                           mass;
    unsigned int                    collisions;

    struct SoilComposition
    {
        float water;
        float soil;
    } soil;
    struct AtmosphericComposition
    {
        float water;
        float soil;
    } atmosphere;

    struct Material {
        DirectX::SimpleMath::Vector4 color;
        DirectX::SimpleMath::Vector3 Ka;
        DirectX::SimpleMath::Vector3 Kd;
        DirectX::SimpleMath::Vector3 Ks;
        float alpha;
    } material;

    const DirectX::SimpleMath::Vector3 GetPosition() { return position; }
    double GetScreenSize() { return (radius / S_NORM) * 2.; }
};


