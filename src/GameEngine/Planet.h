#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

class Planet
{
public:
    typedef struct PlanetDescription
    {
        unsigned int                    id;
        DirectX::SimpleMath::Vector3    position;
        DirectX::SimpleMath::Vector3    velocity;
        DirectX::SimpleMath::Vector3    gravity;
        DirectX::SimpleMath::Vector3    tidal;
        float                           radius;
        float                           mass;

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
    };
public:
    Planet(const double mass, const double size, const DirectX::XMVECTORF32 color = DirectX::Colors::White) noexcept(false);
    Planet(const Planet& planet) :
        m_origin(planet.m_origin),
        m_description(planet.m_description) { }
    Planet& operator=(const Planet& planet) = delete;
    
    ~Planet() { };

    void Update(DX::StepTimer const& timer);

    void SetPosition(const DirectX::SimpleMath::Vector3 position, const DirectX::SimpleMath::Vector3 direction, const float velocity) {
        m_origin = position;

        m_description.position = position;
        m_description.velocity = direction * velocity;
    }
    void SetColor(DirectX::XMVECTORF32 color) { m_description.material.color = color; }

    PlanetDescription GetDescription() { return m_description; }

    int GetId() { return m_description.id; }
    DirectX::SimpleMath::Vector3 GetPosition() { return m_description.position; }
    DirectX::SimpleMath::Vector3 GetVelocity() { return m_description.velocity; }
    DirectX::SimpleMath::Vector4 GetColor() { return m_description.material.color; }
    double GetMass() { return m_description.mass; }
    double GetRadius() { return m_description.radius; }
    double GetDiameter() { return m_description.radius * 2.; }
    DirectX::SimpleMath::Vector3 GetTidal() { return m_description.tidal; }
    DirectX::SimpleMath::Vector3 GetGravity() { return m_description.gravity; }

    const Vector3 GetGravitationalAcceleration(Planet& planet);
    const Vector3 GetTidalAcceleration(Planet& planet);

private:
    PlanetDescription                   m_description;
    DirectX::SimpleMath::Vector3        m_origin;
};


