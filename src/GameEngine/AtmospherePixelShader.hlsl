#include "Constants.hlsli"

Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

cbuffer EnvironmentBuffer : register(b1)
{
    float3 light;
    float time;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD;
    float3 world    : POSITION0;
    float3 eye      : POSITION1;
    float3 light    : POSITION2;
    float clouds    : DEPTH;

    Instance instance;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    Instance instance = input.instance;
    Material material = instance.material;
    material.color = float4(1, 1, 1, input.clouds);

    const float formation = 1 - instance.atmosphere.water;
    if (instance.atmosphere.water == 0 || input.clouds < formation) {
        discard;
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(input.eye);
    float3 L = normalize(input.light); // per light source
    float3 R = normalize(2 * max(dot(L, N), 0.f) * N - L); // per light source

    float3 diffuseIntensity = float3(1, 1, 1); // per light source
    float3 specularIntensity = float3(1, 1, 1); // per light source

    float3 Ip = material.Ka;

    float NdotL = dot(N, L);

    float3 diffuse = material.Kd * max(dot(L, N), 0.f) * diffuseIntensity;
    Ip += saturate(diffuse);

    float3 specular = material.Ks * pow(max(dot(R, V), 0.f), material.alpha) * specularIntensity;
    Ip += saturate(specular);

    return material.color * float4(Ip, 1.f);
}