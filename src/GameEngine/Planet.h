#pragma once

#include "Sphere.h"
#include "Buffers.h"
#include "StepTimer.h"

#include <array>
#include <optional>

typedef struct DepthInfo;
typedef struct ColorProfile;

struct Planet
{
public:
    Planet() : id(rand() * rand()) {
        //ZeroMemory(this, sizeof(this));
    }
    Planet(const double mass, double density, double temperature, const Vector3 position, const Vector3 direction, const float velocity) noexcept(false);
    Planet(const Planet& planet) :
        id(planet.id),
        position(planet.position),
        direction(planet.direction),
        velocity(planet.velocity),
        angular(planet.angular),
        radius(planet.radius),
        mass(planet.mass),
        temperature(planet.temperature),
        density(planet.density),
        material(planet.material),
        collisions(planet.collisions),
        collision(planet.collision),
        quadrantMass(planet.quadrantMass) { }

    Planet& operator=(const Planet& planet)
    {
        memcpy(this, &planet, sizeof(planet));
        return *this;
    }
    //Planet& operator=(const Planet& planet) = delete;

    unsigned int const              id;
    DirectX::SimpleMath::Vector3    position;
    DirectX::SimpleMath::Vector3    direction;
    DirectX::SimpleMath::Vector3    velocity;
    DirectX::SimpleMath::Vector3    angular;
    float                           radius;
    float                           mass;
    float                           temperature;
    float                           density;
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

    const DirectX::SimpleMath::Vector3 GetPosition() const { return position; }
    double GetScreenSize() const { return (radius / S_NORM) * 2.; }

    double GetMass() const { return static_cast<double>(mass); }
    double GetRadius() const { return static_cast<double>(radius); }
    double GetVolume() const { return pow(GetRadius(), 3) * PI_CB; }
    double GetDensity() const { return GetMass() / GetVolume(); }

    std::vector<DepthInfo>& GetDensityProfile();
    void RefreshDensityProfile();
    const void Update(float const deltaTime);
    std::optional<double> RadiusByDensity();
    std::optional<double> MassByDensity();

    static const float RadiusByMass(double mass);
};

template<typename  T>
struct Composition
{
    T Hydrogen;
    T Helium;
    T Lithium;
    T Beryllium;
    T Boron;
    T Carbon;
    T Nitrogen;
    T Oxygen;
    T Fluorine;
    T Neon;
    T Sodium;
    T Magnesium;
    T Aluminum;
    T Silicon;
    T Phosphorus;
    T Sulfur;
    T Chlorine;
    T Argon;
    T Potassium;
    T Calcium;
    T Scandium;
    T Titanium;
    T Vanadium;
    T Chromium;
    T Manganese;
    T Iron;
    T Cobalt;
    T Nickel;
    T Copper;
    T Zinc;
    T Gallium;
    T Germanium;
    T Arsenic;
    T Selenium;
    T Bromine;
    T Krypton;
    T Rubidium;
    T Strontium;
    T Yttrium;
    T Zirconium;
    T Niobium;
    T Molybdenum;
    T Technetium;
    T Ruthenium;
    T Rhodium;
    T Palladium;
    T Silver;
    T Cadmium;
    T Indium;
    T Tin;
    T Antimony;
    T Tellurium;
    T Iodine;
    T Xenon;
    T Cesium;
    T Barium;
    T Lanthanum;
    T Cerium;
    T Praseodymium;
    T Neodymium;
    T Promethium;
    T Samarium;
    T Europium;
    T Gadolinium;
    T Terbium;
    T Dysprosium;
    T Holmium;
    T Erbium;
    T Thulium;
    T Ytterbium;
    T Lutetium;
    T Hafnium;
    T Tantalum;
    T Tungsten;
    T Rhenium;
    T Osmium;
    T Iridium;
    T Platinum;
    T Gold;
    T Mercury;
    T Thallium;
    T Lead;
    T Bismuth;
    T Polonium;
    T Astatine;
    T Radon;
    T Francium;
    T Radium;
    T Actinium;
    T Thorium;
    T Protactinium;
    T Uranium;
    T Neptunium;
    T Plutonium;
    T Americium;
    T Curium;
    T Berkelium;
    T Californium;
    T Einsteinium;
    T Fermium;
    T Mendelevium;
    T Nobelium;
    T Lawrencium;
    T Rutherfordium;
    T Dubnium;
    T Seaborgium;
    T Bohrium;
    T Hassium;
    T Meitnerium;

    //const void Normalize();
    const void Randomize(const Planet& planet);
    const DirectX::SimpleMath::Vector4 GetColor();

    T* data() { return (T*)this; }
    size_t size() const { return sizeof(Composition<T>) / sizeof(T); }
};

struct DepthInfo
{
    double radius;
    double area;
    double volume;
    double mass;
    double density;
    double pressure;
    double temperature;

    Composition<double> composition;
};

struct ElementInfo
{
    ElementInfo(uint32_t Z, double weight, double density, double mass) :
        Z(Z), weight(weight), density(density), mass(mass) { }

    uint32_t Z;
    double weight;
    double density;
    double mass;
};

struct PlanetDescription
{
    //PlanetDescription(Planet planet, Composition composition) : 
    //    planet(planet), 
    //    composition(composition) {};

    Planet planet;
    Composition<float> composition;
};