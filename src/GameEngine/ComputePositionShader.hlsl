#include "Physics.hlsli"

RWStructuredBuffer<Instance> instances : register(u0);

cbuffer EnvironmentBuffer : register(b0)
{
    float3 light;
    float deltaTime;
    float totalTime;
};

[numthreads(1, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    const uint current = dispatchThreadId.x;

    Instance body = instances[current];

    // Do not update the central body
    if ((body.center.x + body.center.y + body.center.z) != 0)
    {
        body.center += lerp(float3(0, 0, 0), body.velocity, deltaTime);

        float3 direction = _rotate(body.direction, body.angular);
        body.direction = lerp(body.direction, direction, deltaTime);
    }

    instances[current] = body;
}