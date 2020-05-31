#pragma once

struct Material
{
    float4 color    : INST_MATERIAL_COLOR;
    float3 Ka       : INST_MATERIAL_KA;
    float3 Kd       : INST_MATERIAL_KD;
    float3 Ks       : INST_MATERIAL_KS;
    float alpha : INST_MATERIAL_ALPHA;
};

struct SoilComposition
{
    float water : INST_COMPOSITION_SOIL_A;
    float soil : INST_COMPOSITION_SOIL_B;
};

struct AtmosphereComposition
{
    float water : INST_COMPOSITION_ATMOSPHERE_A;
    float soil : INST_COMPOSITION_ATMOSPHERE_B;
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
    uint collisions : INST_COLLISIONS;

    SoilComposition soil;
    AtmosphereComposition atmosphere;
    Material material;
};

struct CollisionInfo {
    double3 velocity1;
    double3 velocity2;
    double3 angular1;
    double3 angular2;
};