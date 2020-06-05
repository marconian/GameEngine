#pragma once

struct Material
{
    float4 color    : INST_MATERIAL_COLOR;
    float3 Ka       : INST_MATERIAL_KA;
    float3 Kd       : INST_MATERIAL_KD;
    float3 Ks       : INST_MATERIAL_KS;
    float alpha : INST_MATERIAL_ALPHA;
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
};

struct Instance
{
    uint id         : INST_ID;
    float3 center   : INST_POSITION;
    float3 direction : INST_DIRECTION;
    float3 velocity : INST_VELOCITY;
    float3 angular : INST_ANGULAR;
    float3 gravity  : INST_GRAVITY;
    float3 tidal    : INST_TIDAL;
    float radius : INST_RADIUS;
    float mass : INST_MASS;
    float energy : INST_ENERGY;
    uint collision : INT_COLLISION;
    uint collisions : INST_COLLISIONS;
    float quadrantMass : INST_MASS_Q;

    Material material;
};

struct InstanceDescription
{
    Instance instance;
    Composition composition;
};

struct CollisionInfo {
    double3 velocity1;
    double3 velocity2;
    double3 angular1;
    double3 angular2;
};