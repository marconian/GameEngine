#include "Globals.hlsli"

RWStructuredBuffer<InstanceDescription> instances : register(u0);

cbuffer CursorBuffer : register(b0) { uint cursor; };
cbuffer EnvironmentBuffer : register(b1)
{
    float deltaTime;
    float totalTime;
    float3 light;
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

    InstanceDescription body = instances[current];
    InstanceDescription r_body = instances[reference];

    if (current != reference && body.instance.mass != 0 && r_body.instance.mass != 0)
    {
        float radius = (float)toReal(distance(body.instance.center, r_body.instance.center));
        bool collision = radius <= (body.instance.radius + r_body.instance.radius);

        if (collision && body.instance.mass > r_body.instance.mass) {
            CollisionInfo collision = Collision(body.instance, r_body.instance);

            float density = pow(body.instance.mass, 1 / 3.) / body.instance.radius;

            body.instance.mass += r_body.instance.mass;
            body.instance.radius = pow(body.instance.mass, 1 / 3.) / density;
            body.instance.angular = (float3)collision.angular1;
            body.instance.velocity = (float3)collision.velocity1;
            body.instance.collisions += 1;

            body.composition = GetCollisionComposition(body, r_body);
        }
        else
        {
            body.instance.center = float3(0, 0, 0);
            body.instance.mass = 0;

            Composition composition;
            body.composition = composition;
        }


        float c[109] = (float[109])body.composition;
        float s = 0;
        for (int i = 0; i < 109; i++)
            s += c[i];

        for (int i = 0; i < 109; i++)
            c[i] /= s;

        body.composition = (Composition)c;
        //body.instance.material.color = float4(GetCompositionColor(body.composition).xyz, 1);
    }

    body.instance.collision = 0;

    AllMemoryBarrier();

    instances[current] = body;
}