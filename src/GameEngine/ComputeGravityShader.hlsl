#include "Globals.hlsli"

RWStructuredBuffer<Instance> instances : register(u0);

[numthreads(1, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    const uint current = dispatchThreadId.x;
    const uint reference = dispatchThreadId.y;

    Instance body = instances[current];
    Instance r_body = instances[reference];

    if (current != reference && body.id != 0 && r_body.id != 0)
    {
        float radius = distance(body.center, r_body.center) * (float)S_NORM;

        bool collision = false;
        if (radius <= (body.radius + r_body.radius)) {
            if (body.mass > r_body.mass)
            {
                float density = pow(body.mass, 1/3.) / body.radius;

                body.mass += r_body.mass;
                body.radius = pow(body.mass, 1 / 3.) / density;

                CollisionInfo collision = Collision(body, r_body);
                body.velocity = (float3)collision.velocity1;
                body.angular = (float3)collision.angular1;

                r_body.id = 0;
                r_body.center = float3(0, 0, 0);
                r_body.mass = 0;

                instances[reference] = r_body;
            }
            else collision = true;
        }

        // Do not update the central body or smaller body on collision
        if ((body.center.x + body.center.y + body.center.z) != 0 && !collision) // && (r_body.mass / body.mass) > .001
        {
            double3 acceleration = GravitationalAcceleration(body, r_body);

            AllMemoryBarrier();

            body.gravity += (float3)acceleration;
            body.velocity += (float3)acceleration;
        }
    }

    instances[current] = body;
}