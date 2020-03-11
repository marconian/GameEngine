//

static const float S_NORM = 167420.f;

struct Material
{
    float4 color    : INST_MATERIAL_COLOR;
    float3 Ka       : INST_MATERIAL_KA;
    float3 Kd       : INST_MATERIAL_KD;
    float3 Ks       : INST_MATERIAL_KS;
    float alpha     : INST_MATERIAL_ALPHA;
};

struct Composition
{
    float water     : INST_COMPOSITION_WATER;
    float soil      : INST_COMPOSITION_SOIL;
};

struct Instance
{
    uint id         : INST_ID;
    float3 center   : INST_POSITION;
    float radius    : INST_RADIUS;

    Material material;
    Composition composition;
};

float scale(float value, float limit)
{
    return min(max(value, -limit), limit);
}