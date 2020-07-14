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
    float deltaTime;
    float totalTime;
    float3 light;
};

struct VS_INPUT
{
    float3 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 tex      : TEXCOORD;

    Instance instance;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 tex      : TEXCOORD;
    float3 eye      : POSITION0;
    float3 light    : POSITION1;
    float3 local    : POSITION2;
    float level : DEPTH;

    Instance instance;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    Instance instance = input.instance;

    float3 _center = instance.center;
    float3 _position = _rotate(input.position, instance.direction) + _center;
    float3 _normal = _rotate(input.normal, instance.direction);
    float3 _eye = eye - _position;
    float3 _light = light - _position;
    float _level = toReal(distance(float3(0, 0, 0), input.position) - toScreen(instance.radius));

    output.position = mul(float4(_position, 1.), mvp);
    output.normal = mul(float4(_normal, 1.), m).xyz;
    output.color = input.color;
    output.eye = mul(float4(_eye, 1.), m).xyz;
    output.light = mul(float4(_light, 1.), m).xyz;
    output.tex = input.tex;
    output.level = _level;
    output.local = input.position;

    output.instance = instance;

    return output;
}