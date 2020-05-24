#include "Physics.hlsli"

RWStructuredBuffer<Instance> instances : register(u0);

//cbuffer InstanceBuffer : register(b0)
//{
//    InstanceSimple instances[16];
//};

[numthreads(16, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    const uint current = groupId.x;

    Instance body = instances[current];

    double3 acceleration = double3(0, 0, 0);
    
    if (body.id != 0 && (float)body.center != 0) {
    
        for (int i = 0; i < 16; i++) {
            Instance r_body = instances[i];
            if (i != current && r_body.id != 0) {
                acceleration += GravitationalAcceleration(body, r_body);
            }
        }
    }

    body.gravity = (float3)acceleration;
    body.velocity += body.gravity;

    instances[current] = body;
    //real[current] = float3(current, body.id, body.mass);
}