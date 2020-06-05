#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

struct Planet
{
public:
    Planet() {}
    Planet(const double mass, const Vector3 position, const Vector3 direction, const float velocity) noexcept(false);
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
        energy(planet.energy),
        material(planet.material),
        collisions(planet.collisions),
        collision(planet.collision),
        quadrantMass(planet.quadrantMass) { }
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
    float                           energy;
    unsigned int                    collision;
    unsigned int                    collisions;
    float                           quadrantMass;

    struct Material {
        DirectX::SimpleMath::Vector4 color;
        DirectX::SimpleMath::Vector3 Ka;
        DirectX::SimpleMath::Vector3 Kd;
        DirectX::SimpleMath::Vector3 Ks;
        float alpha;
    } material;

    const std::string GetQuadrant();

    const DirectX::SimpleMath::Vector3 GetPosition() { return position; }
    double GetScreenSize() { return (radius / S_NORM) * 2.; }
};

struct Composition
{
    float Hydrogen;
    float Helium;
    float Lithium;
    float Beryllium;
    float Boron;
    float Carbon;
    float Nitrogen;
    float Oxygen;
    float Fluorine;
    float Neon;
    float Sodium;
    float Magnesium;
    float Aluminum;
    float Silicon;
    float Phosphorus;
    float Sulfur;
    float Chlorine;
    float Argon;
    float Potassium;
    float Calcium;
    float Scandium;
    float Titanium;
    float Vanadium;
    float Chromium;
    float Manganese;
    float Iron;
    float Cobalt;
    float Nickel;
    float Copper;
    float Zinc;
    float Gallium;
    float Germanium;
    float Arsenic;
    float Selenium;
    float Bromine;
    float Krypton;
    float Rubidium;
    float Strontium;
    float Yttrium;
    float Zirconium;
    float Niobium;
    float Molybdenum;
    float Technetium;
    float Ruthenium;
    float Rhodium;
    float Palladium;
    float Silver;
    float Cadmium;
    float Indium;
    float Tin;
    float Antimony;
    float Tellurium;
    float Iodine;
    float Xenon;
    float Cesium;
    float Barium;
    float Lanthanum;
    float Cerium;
    float Praseodymium;
    float Neodymium;
    float Promethium;
    float Samarium;
    float Europium;
    float Gadolinium;
    float Terbium;
    float Dysprosium;
    float Holmium;
    float Erbium;
    float Thulium;
    float Ytterbium;
    float Lutetium;
    float Hafnium;
    float Tantalum;
    float Tungsten;
    float Rhenium;
    float Osmium;
    float Iridium;
    float Platinum;
    float Gold;
    float Mercury;
    float Thallium;
    float Lead;
    float Bismuth;
    float Polonium;
    float Astatine;
    float Radon;
    float Francium;
    float Radium;
    float Actinium;
    float Thorium;
    float Protactinium;
    float Uranium;
    float Neptunium;
    float Plutonium;
    float Americium;
    float Curium;
    float Berkelium;
    float Californium;
    float Einsteinium;
    float Fermium;
    float Mendelevium;
    float Nobelium;
    float Lawrencium;
    float Rutherfordium;
    float Dubnium;
    float Seaborgium;
    float Bohrium;
    float Hassium;
    float Meitnerium;

    const void Normalize();
    const void Randomize();
    const void Degenerate(Planet& planet);
    const double GetAtmosphericMass(const Planet& planet);
    const DirectX::SimpleMath::Vector4 GetColor();
};

struct PlanetDescription
{
    //PlanetDescription(Planet planet, Composition composition) : 
    //    planet(planet), 
    //    composition(composition) {};

    Planet planet;
    Composition composition;
};