#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

class Planet
{
public:
    Planet(const double mass, const double size, const DirectX::XMVECTORF32 color = DirectX::Colors::White) noexcept(false);
    Planet(const Planet& planet) :
        m_id(planet.m_id),
        m_color(planet.m_color),
        m_mass(planet.m_mass),
        m_origin(planet.m_origin),
        m_position(planet.m_position),
        m_rotation(planet.m_rotation),
        m_size(planet.m_size),
        m_velocity(planet.m_velocity) { }
    Planet& operator=(const Planet& planet) = delete;
    
    ~Planet() { };

    void Update(DX::StepTimer const& timer);

    void SetPosition(const DirectX::SimpleMath::Vector3 position, const DirectX::SimpleMath::Vector3 direction, const float velocity) {
        m_position = position;
        m_origin = position;
        m_velocity = direction * velocity;
    }
    void SetColor(DirectX::XMVECTORF32 color) { m_color = color; }

    int GetId() { return m_id; }
    DirectX::SimpleMath::Vector3 GetPosition() { return m_position; }
    DirectX::SimpleMath::Vector3 GetVelocity() { return m_velocity; }
    DirectX::SimpleMath::Vector4 GetColor() { return m_color; }
    double GetMass() { return m_mass; }
    double GetDiameter() { return m_size; }
    double GetSize() { return m_size; }
    DirectX::SimpleMath::Vector3 GetTidal() { return m_tidal; }
    DirectX::SimpleMath::Vector3 GetGravity() { return m_gravity; }

    const Vector3 GetGravitationalAcceleration(Planet& planet);
    const Vector3 GetTidalAcceleration(Planet& planet);

private:

    int                                                         m_id;
    DirectX::SimpleMath::Vector3                                m_position;
    DirectX::SimpleMath::Vector3                                m_origin;
    DirectX::SimpleMath::Vector3                                m_velocity;
    DirectX::SimpleMath::Vector3                                m_gravity;
    DirectX::SimpleMath::Vector3                                m_tidal;
    DirectX::SimpleMath::Quaternion                             m_rotation;
    DirectX::XMVECTORF32                                        m_color;
    double                                                      m_size;
    double                                                      m_mass;
};


