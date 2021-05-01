#pragma once

#include "StepTimer.h"

#include <array>
#include <optional>

typedef struct DepthInfo;
typedef struct ColorProfile;

struct Planet
{
public:
	Planet() : id(rand() * rand())
	{
		//ZeroMemory(this, sizeof(this));
	}

	Planet(double mass, double density, double temperature, DirectX::SimpleMath::Vector3 position,
	       DirectX::SimpleMath::Vector3 direction, float velocity) noexcept(false);

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
		collision(planet.collision),
		collisions(planet.collisions),
		quadrantMass(planet.quadrantMass),
		material(planet.material)
	{
	}

	Planet& operator=(const Planet& planet)
	{
		memcpy(this, &planet, sizeof(planet));
		return *this;
	}

	unsigned int const id;
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 direction;
	DirectX::SimpleMath::Vector3 velocity;
	DirectX::SimpleMath::Vector3 angular;
	float radius;
	float mass;
	float temperature;
	float density;
	unsigned int collision = false;
	unsigned int collisions = 0;
	float quadrantMass = 0;

	struct Material
	{
		DirectX::SimpleMath::Vector4 color;
		DirectX::SimpleMath::Vector3 Ka;
		DirectX::SimpleMath::Vector3 Kd;
		DirectX::SimpleMath::Vector3 Ks;
		float alpha;
	} material{};

	std::string GetQuadrant() const;

	const DirectX::SimpleMath::Vector3 GetPosition() const { return position; }
	double GetScreenSize() const { return (radius / S_NORM) * 2.; }

	double GetMass() const { return static_cast<double>(mass); }
	double GetRadius() const { return static_cast<double>(radius); }
	double GetVolume() const { return pow(GetRadius(), 3) * PI_CB; }
	double GetDensity() const { return GetMass() / GetVolume(); }

	std::vector<DepthInfo>& GetDensityProfile();
	void RefreshDensityProfile() const;
	void Update(float deltaTime);
	std::optional<double> RadiusByDensity();
	std::optional<double> MassByDensity();

	static float RadiusByMass(double mass);
};

template <typename T>
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
	void Randomize(const Planet& planet);
	[[nodiscard]] DirectX::SimpleMath::Vector4 GetColor() const;

	T* data() const { return (T*)this; }
	static size_t size() { return sizeof(Composition<T>) / sizeof(T); }

	T sum() const
	{
		T s = 0;
		for (int i = 0; i < size(); i++)
			s += data()[i];
		return s;
	}

	template <typename A>
	[[nodiscard]] Composition<A>& As() const
	{
		auto* a = new Composition<A>();
		for (auto i = 0; i < size(); i++)
			a->data()[i] = data()[i];
		return *a;
	}

	template <typename A>
	Composition<T>& operator=(const Composition<A>& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] = ((T*)&value)[i];

		return *this;
	}

	template <typename A, size_t S>
	Composition<T>& operator=(const std::array<A, S>& value)
	{
		for (int i = 0; i < S; i++)
			data()[i] = ((T*)&value)[i];

		return *this;
	}

	Composition<T>& operator=(const T& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] = value;

		return *this;
	}

	template <typename A>
	Composition<T>& operator+=(const Composition<A>& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] += ((T*)&value)[i];

		return *this;
	}

	template <typename A>
	friend Composition<T>& operator+(Composition<T> lhs, const Composition<A>& rhs)
	{
		lhs += rhs;
		return lhs;
	}

	template <typename A>
	Composition<T>& operator-=(const Composition<A>& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] -= ((T*)&value)[i];

		return *this;
	}

	template <typename A>
	friend Composition<T>& operator-(Composition<T> lhs, const Composition<A>& rhs)
	{
		lhs -= rhs;
		return lhs;
	}

	template <typename A>
	Composition<T>& operator*=(const A& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] *= static_cast<T>(value);

		return *this;
	}

	template <typename A>
	friend Composition<T>& operator*(Composition<T> lhs, const A& rhs)
	{
		lhs *= rhs;
		return lhs;
	}

	template <typename A>
	Composition<T>& operator/=(const A& value)
	{
		for (int i = 0; i < size(); i++)
			data()[i] /= static_cast<T>(value);

		return *this;
	}

	template <typename A>
	friend Composition<T>& operator/(Composition<T> lhs, const A& rhs)
	{
		lhs /= rhs;
		return lhs;
	}
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

	DepthInfo& operator+=(const DepthInfo& value)
	{
		composition += value.composition;
		mass = composition.sum();
		volume = mass / density;

		return *this;
	}

	DepthInfo& operator+=(const Composition<double>& value)
	{
		composition += value;
		mass = composition.sum();
		volume = mass / density;

		return *this;
	}

	friend DepthInfo& operator+(DepthInfo lhs, const DepthInfo& rhs)
	{
		lhs += rhs;
		return lhs;
	}
};

template <typename T, size_t S>
struct PlanetProfile
{
	PlanetProfile<T, S>()
	{
		size_t a = 1;
		size_t b = S;
		size_t c = pow(S, 2);

		data = calloc(pow(S, 3), sizeof(T));
	}

	PlanetProfile<T, S>(const PlanetProfile<T, S>& ref) :
		data(ref.data)
	{
	}

	PlanetProfile<T, S>& operator=(const PlanetProfile<T, S>& ref)
	{
		memcpy(this, &ref, pow(S, 3) * sizeof(T));
		return *this;
	}

	~PlanetProfile<T, S>() { free(data); }

private:
	T* data;
};

struct ElementInfo
{
	ElementInfo(uint32_t Z, double weight, double density, double mass) :
		Z(Z), weight(weight), density(density), mass(mass)
	{
	}

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
