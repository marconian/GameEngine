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

    Instance body = instances[current];

    if (body.mass != 0)
    {
        //body.center -= centerOfMass;

        if (body.quadrantMass > 0)
        {
            Instance r_body;
            r_body.center = float3(0, 0, 0);
            r_body.mass = (systemMass - body.quadrantMass);

            double3 acceleration = GravitationalAcceleration(body, r_body, deltaTime);
            body.velocity += (float3)acceleration;
        }

        body.center += lerp(float3(0, 0, 0), body.velocity, deltaTime);
        body.direction += lerp(float3(0, 0, 0), body.angular, deltaTime);

        double3 velocityVector = toReal((double3)body.velocity);
        //double3 angularVector = toReal((double3)body.angular);

        double velocity = _pow(velocityVector.x, 2) + _pow(velocityVector.y, 2) + _pow(velocityVector.z, 2);

        /*double angularVelocity = _pow(angularVector.x, 2) + _pow(angularVector.y, 2) + _pow(angularVector.z, 2);
        double3x3 inertiaTensor = InertiaTensor(body.direction, body.mass);
        double moment = _dot(body.direction, mul(body.direction, inertiaTensor));
        double angularEnergy = .5 * moment * _pow(angularVelocity, 2);*/

        instances[current] = body;
    }
}