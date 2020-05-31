#include "Globals.hlsli"

cbuffer ModelViewProjectionBuffer : register(b0)
{
    matrix m;
    matrix v;
    matrix p;
    matrix mv;
    matrix mp;
    matrix vp;
    matrix mvp;
    float3 eye;
};

cbuffer EnvironmentBuffer : register(b1)
{
    float3 light;
    float deltaTime;
    float totalTime;
};

struct VS_INPUT
{
    float3 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;

    Instance instance;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;
    float3 light    : POSITION0;

    Instance instance;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    Instance instance = input.instance;

    float3 _center = instance.center;
    float _radius = toScreen(instance.radius);
    float3 _model = input.position * _radius;
    float3 _position = _model + _center;
    float3 _light = normalize(light - _center);

    output.position = mul(float4(_position, 1.), mvp);
    output.normal = mul(float4(input.normal, 1.), m).xyz;
    output.light = mul(float4(_light, 1.), m).xyz;
    output.tex = input.tex;

    output.instance = instance;

    return output;
}