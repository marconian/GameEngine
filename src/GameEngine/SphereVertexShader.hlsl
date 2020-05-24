#include "Globals.hlsli"

RWStructuredBuffer<float> real : register(u0);

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
    float time;
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
    float3 world    : POSITION0;
    float3 eye      : POSITION1;
    float3 light    : POSITION2;

    Instance instance;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    Instance instance = input.instance;

    float3 _center = instance.center + real[1];
    float3 _radius = TerrainLevel(instance, input.position);
    float3 _model = input.position * _radius;
    float3 _position = _model + _center;
    float3 _eye = normalize(eye - _position);
    float3 _light = normalize(light - _position);

    //_position += normalize(input.position) * _noise;

    output.position = mul(float4(_position, 1.), mvp);
    output.world = mul(float4(_position, 1.), m).xyz;
    output.normal = mul(float4(input.normal, 1.), m).xyz;
    output.eye = mul(float4(_eye, 1.), m).xyz;
    output.light = mul(float4(_light, 1.), m).xyz;
    output.tex = input.tex;

    output.instance = instance;

    //float4 _color = output.instance.material.color;
    //float3 _val = _noise * _color.xyz;
    //output.instance.material.color = float4(normalize(_val), _color.w);

	return output;
}