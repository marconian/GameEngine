#include "Globals.hlsli"

RWStructuredBuffer<Instance> instances : register(u0);

cbuffer CursorBuffer : register(b0) { uint cursor; };
cbuffer EnvironmentBuffer : register(b1)
{
    float deltaTime;
    float totalTime;
    float3 light;
};
cbuffer SystemBuffer : register(b2)
{
    float systemMass;
    float3 centerOfMass;
};

[numthreads(1, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    const uint current = dispatchThreadId.x;
    const uint reference = dispatchThreadId.y + cursor;

    Instance body = instances[current];
    Instance r_body = instances[reference];

    if (current != reference && body.mass != 0 && r_body.mass != 0)
    {
        float radius = (float)toReal(distance(body.center, r_body.center));

        // perform basic collision detection. Handled further by the ComputeCollisionShader
        if (radius <= (body.radius + r_body.radius)) {
            body.collision = 1;
        }

        double3 acceleration = GravitationalAcceleration(body, r_body, deltaTime);
        body.velocity += (float3)acceleration;

        //AllMemoryBarrier();

        instances[current] = body;
    }
}