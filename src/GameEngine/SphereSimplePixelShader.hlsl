#include "Globals.hlsli"

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 tex      : TEXCOORD;
    float3 light    : POSITION0;

    Instance instance;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    Instance instance = input.instance;
    Material material = instance.material;

    float3 N = normalize(input.normal);
    float3 L = normalize(input.light); // per light source

    float3 Ip = material.Ka;

    float3 diffuse = material.Kd * max(dot(L, N), 0.f);
    Ip += saturate(diffuse);

    return material.color * float4(Ip, 1.f);
}