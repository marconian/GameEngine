#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

class Planet
{
public:
    typedef struct PlanetDescriptionSimple {
        unsigned int                    id;
        float                           mass;
        XMFLOAT3                        position;
    };

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
    Planet(const double mass, const double radius, const DirectX::XMVECTORF32 color = DirectX::Colors::White) noexcept(false);
    Planet(const Planet& planet) :
        m_origin(planet.m_origin),
        m_description(planet.m_description) { }
    Planet& operator=(const Planet& planet) = delete;
    
    ~Planet() { };

    void Update(DX::StepTimer const& timer, Planet::PlanetDescription description);

    void SetPosition(const DirectX::SimpleMath::Vector3 position, const DirectX::SimpleMath::Vector3 direction, const float velocity) {
        m_origin = position;

        m_description.position = position;
        m_description.velocity = direction * velocity;
    }
    void SetColor(DirectX::XMVECTORF32 color) { m_description.material.color = color; }

    PlanetDescription GetDescription() { return m_description; }
    PlanetDescriptionSimple GetDescriptionSimple() {
        PlanetDescriptionSimple description{};
        description.id = m_description.id;
        description.mass = m_description.mass;
        description.position = m_description.position;

        return description;
    }

    DirectX::SimpleMath::Vector3 GetPosition() { return m_description.position; }
    double GetScreenSize() { return (m_description.radius / S_NORM) * 2.; }

private:
    PlanetDescription                   m_description;
    DirectX::SimpleMath::Vector3        m_origin;
};


