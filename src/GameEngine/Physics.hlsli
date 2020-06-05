#pragma once

#include "Globals.hlsli"
#include "Spectra.hlsli"

double3 GravitationalAcceleration(Instance body1, Instance body2, double deltaTime)
{
    double3 center1 = (double3)body1.center;
    double3 center2 = (double3)body2.center;

    double radius = toReal(_distance((double3)body1.center, (double3)body2.center));

    double strength = deltaTime;

    if (radius > 0) {
        double mass = (double)body1.mass;
        double mass_p = (double)body2.mass;

        double g_force = (G * mass * mass_p) / _pow(radius, 2);
        double acceleration = toScreen(g_force * strength / mass);

        double3 g_vector = _normalize(center2 - center1);
        g_vector *= acceleration;

        return g_vector;
    }

    return double3(0, 0, 0);
}

double3 TidalForce(double3 position, double3 radius, double3 tidal)
{
    double3 _tidal = tidal * radius * 2;
    double3 _pull = _normalize(position) * abs(_tidal);

    //float3 _pull = normalize(input.position) * abs(normalize(pull));
    //float _max_pull = max(abs(_pull), 0);
    //if (_max_pull > 0)
    //    _position += _pull * .1f;

    return _pull;
}

//const Vector3 Planet::GetTidalAcceleration(Planet& planet)
//{
//    Vector3 p_relative = m_description.position - planet.GetPosition();
//
//    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
//    if (radius > 0)
//    {
//        const long double mass = m_description.mass;
//        const long double radius_m = radius * S_NORM;
//
//        const long double t_force = G * (mass / pow(radius_m, 3));
//        const long double acceleration = t_force / S_NORM;
//
//        Vector3 t_vector;
//        (-p_relative).Normalize(t_vector);
//
//        t_vector *= static_cast<float>(acceleration);
//
//        return t_vector;
//    }
//
//    return Vector3::Zero;
//}

//This function calulates the velocities after a 3D collision vaf, vbf, waf and wbf from information about the colliding bodies
//    @param double e coefficient of restitution which depends on the nature of the two colliding materials
//    @param double ma total mass of body a
//    @param double mb total mass of body b
//    @param matrix Ia inertia tensor for body a in absolute coordinates(if this is known in local body coordinates it must
//        be converted before this is called).
//    @param matrix Ib inertia tensor for body b in absolute coordinates(if this is known in local body coordinates it must
//        be converted before this is called).
//    @param vector ra position of collision point relative to centre of mass of body a in absolute coordinates(if this is
//        known in local body coordinates it must be converted before this is called).
//    @param vector rb position of collision point relative to centre of mass of body b in absolute coordinates(if this is
//        known in local body coordinates it must be converted before this is called).
//    @param vector n normal to collision point, the line along which the impulse acts.
//    @param vector vai initial velocity of centre of mass on object a
//    @param vector vbi initial velocity of centre of mass on object b
//    @param vector wai initial angular velocity of object a
//    @param vector wbi initial angular velocity of object b
CollisionInfo CollisionResponce(double3x3 Ia, double3x3 Ib, double e, double3 ra, double3 rb, double ma, double mb, double3 vai, double3 vbi, double3 wai, double3 wbi, double3 normal)
{
    double3 angularA = _mul(_cross(normal, ra), _inverse(Ia)); // start calculating the change in angular rotation of a
    double3 angularB = _mul(_cross(normal, rb), _inverse(Ib)); // start calculating the change in abgular rotation of b

    double3 velocityA = _cross(angularA, ra);  // calculate the linear velocity of collision point on a due to rotation of a
    double3 velocityB = _cross(angularB, rb);  // calculate the linear velocity of collision point on b due to rotation of b

    double3 velocityR = vbi - vai; // relative velocity

    double scalar = (1 / ma + _dot(velocityA, normal)) + (1 / mb + _dot(velocityB, normal));
    double mag = _sqrt(_pow(velocityR.x, 2) + _pow(velocityR.y, 2) + _pow(velocityR.z, 2));

    double Jmod = (e + 1) * mag / scalar;
    double3 J = mul(normal, Jmod);

    CollisionInfo info;
    info.velocity1 = vai - mul(J, 1 / ma);
    info.velocity2 = vbi - mul(J, 1 / mb);
    info.angular1 = wai - angularA;
    info.angular2 = wbi - angularB;

    return info;
}

double3x3 InertiaTensor(double3 position, double mass)
{
    double Ixx = mass * (_pow(position.y, 2) + _pow(position.z, 2));
    double Iyy = mass * (_pow(position.z, 2) + _pow(position.x, 2));
    double Izz = mass * (_pow(position.x, 2) + _pow(position.y, 2));
    double Ixy = -(mass * position.x * position.y);
    double Ixz = -(mass * position.x * position.z);
    double Iyz = -(mass * position.y * position.z);

    double3x3 I = {
        Ixx, Ixy, Ixz,
        Ixy, Iyy, Iyz,
        Ixz, Iyz, Izz
    };

    return I;
}

CollisionInfo Collision(Instance body1, Instance body2)
{
    double3 pos1 = toReal((double3)body1.center);
    double3 pos2 = toReal((double3)body2.center);

    double3 vel1 = toReal((double3)body1.velocity);
    double3 vel2 = toReal((double3)body2.velocity);

    double3 dir1 = _normalize((double3)body1.velocity);
    double3 dir2 = _normalize((double3)body2.velocity);

    double mass1 = (double)body1.mass;
    double mass2 = (double)body2.mass;

    double ang1 = (double)body1.angular;
    double ang2 = (double)body2.angular;

    double3x3 Ia = InertiaTensor(pos1, mass1);
    double3x3 Ib = InertiaTensor(pos2, mass2);

    double3 dir = _normalize(pos2 - pos1);
    double3 normal = dir1 + (dir1 - dir2) * .5;

    double e; // coefficient of restitution
    if (mass1 > mass2)
        e = mass2 / mass1;
    else e = mass1 / mass2;

    CollisionInfo info;
    info = CollisionResponce(Ia, Ib, .3, 
        pos1 + dir * (double)body1.radius,
        pos2 + -dir * (double)body2.radius,
        mass1, mass2,
        vel1, vel2,
        ang1, ang2,
        normal
    );

    info.velocity1 = toScreen(info.velocity1);
    info.velocity2 = toScreen(info.velocity2);

    if (isnan((float)info.velocity1.x)) { info.velocity1 = double3(0, 0, 0); }
    if (isnan((float)info.velocity2.x)) { info.velocity1 = double3(0, 0, 0); }
    if (isnan((float)info.angular1.x)) { info.angular1 = double3(0, 0, 0); }
    if (isnan((float)info.angular2.x)) { info.angular2 = double3(0, 0, 0); }

    return info;
}

Composition GetCollisionComposition(InstanceDescription description1, InstanceDescription description2)
{
    Composition composition;
    if (description1.instance.mass == 0 && description2.instance.mass == 0)
        return composition;
    else if (description1.instance.mass == 0)
        return description2.composition;
    else if (description2.instance.mass == 0)
        return description1.composition;
    else
    {
        float a[109] = (float[109])description1.composition;
        float b[109] = (float[109])description2.composition;

        float c[109];
        for (int i = 0; i < 109; i++)
            c[i] = description1.instance.mass * a[i] + description2.instance.mass * b[i];

        composition = (Composition)c;

    }

    return composition;
}

float3 GetCompositionColor(Composition composition)
{
    float3 color = float3(0, 0, 0);

    float c[109] = (float[109])composition;
    for (int i = 0; i < 109; i++)
        color += (ATOM_COLORS[i].xyz) * c[i];

    //color = color * (1 / 109.);

    return normalize(color);
}