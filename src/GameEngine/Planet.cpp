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

Planet::Planet(const double mass, const double radius, const Vector3 position, const Vector3 direction, const float velocity, const XMVECTORF32 color) :
    id(rand()),
    position(position),
    direction(Vector3::Zero),
    velocity(direction * velocity),
    angular(randv(0, 1) * rand(1e-3, 1e-6)),
    gravity(Vector3::Zero),
    tidal(Vector3::Zero),
    radius(radius),
    mass(mass),
    soil(),
    atmosphere(),
    material(),
    collisions(0)
{
    material.color = color;
    material.Ka = Vector3(.03, .03, .03); // Ambient reflectivity
    material.Kd = Vector3(0.1086f, 0.1086f, 0.1086f); // Diffuse reflectivity
    material.Ks = Vector3(.001f, .001f, .001f); // Spectral reflectivity  //Vector4(0.23529f, 0.15686f, 0.07843f, 1.f);
    material.alpha = 0.f;

    atmosphere.water = .4f;
    atmosphere.soil = .5f;

    soil.water = .5f;
    soil.soil = .5f;

    if (mass > SUN_MASS * .5)
    {
        material.Ka = Vector3(1);
        atmosphere.water = 0.f;
    }
}