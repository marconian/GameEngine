#pragma once

#include "Globals.hlsli"

float TerrainLevel(Instance instance, float3 position, bool detailed) {
    float level = (float)(instance.radius * S_NORM_INV);

    if (detailed)
    {
        float _limit = S_NORM_INV * 1000;
        float _noise = scale(fractal(10,
            position.x + instance.id,
            position.y + instance.id,
            position.z + instance.id
        ), _limit);

        level += _noise;
    }

    return level;
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

double3 GravitationalAcceleration(Instance body1, Instance body2)
{
    double3 center1 = (double3)body1.center;
    double3 center2 = (double3)body2.center;

    double radius = _distance((double3)body1.center, (double3)body2.center) * S_NORM * 1000;

    if (radius > 0) {
        double mass = (double)body1.mass;
        double mass_p = (double)body2.mass;

        double g_force = (G * mass * mass_p) / _pow(radius, 2);
        double acceleration = (g_force / mass) * S_NORM_INV;

        double3 g_vector = _normalize(center2 - center1);
        g_vector *= acceleration;

        return g_vector;
    }

    return double3(0, 0, 0);
}

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
CollisionInfo CollisionResponce(double e, double ma, double mb, double3x3 Ia, double3x3 Ib, double3 ra, double3 rb, double3 normal, double3 vai, double3 vbi, double3 wai, double3 wbi)
{
    double3 angularVelChangea = _cross(normal, ra); // start calculating the change in angular rotation of a
    angularVelChangea = _mul(angularVelChangea, _inverse(Ia)); // transform?
    double3 vaLinDueToR = _cross(angularVelChangea, ra);  // calculate the linear velocity of collision point on a due to rotation of a
    double scalar = 1 / ma + _dot(vaLinDueToR, normal);

    double3 angularVelChangeb = _cross(normal, rb); // start calculating the change in abgular rotation of b
    angularVelChangeb = _mul(angularVelChangeb, _inverse(Ib)); // transform?
    double3 vbLinDueToR = _cross(angularVelChangeb, rb);  // calculate the linear velocity of collision point on b due to rotation of b
    scalar += 1 / mb + _dot(vbLinDueToR, normal);

    double3 vi = vai - vbi;
    double mag = _sqrt(_pow(vi.x, 2) + _pow(vi.y, 2) + _pow(vi.z, 2));

    double Jmod = (e + 1) * mag / scalar;
    double3 J = mul(normal, Jmod);

    CollisionInfo info;
    info.velocity1 = vai - mul(J, 1 / ma);
    info.velocity2 = vbi - mul(J, 1 / mb);
    info.angular1 = wai - angularVelChangea;
    info.angular2 = wbi - angularVelChangeb;

    return info;
}

double3x3 InertiaTensor(double3 position, double mass)
{
    double Ixx = mass * (_pow(position.y, 2) + _pow(position.z, 2));
    double Iyy = mass * (_pow(position.x, 2) + _pow(position.z, 2));
    double Izz = mass * (_pow(position.y, 2) + _pow(position.y, 2));
    double Ixy = (-mass) * position.x * position.y;
    double Ixz = (-mass) * position.x * position.z;
    double Iyz = (-mass) * position.y * position.z;

    double3x3 I = {
        Ixx, Ixy, Ixz,
        Ixy, Iyy, Iyz,
        Ixz, Iyz, Izz
    };

    return I;
}

CollisionInfo Collision(Instance body1, Instance body2)
{
    double3 pos1 = (double3)body1.center * S_NORM;
    double3 pos2 = (double3)body2.center * S_NORM;

    double mass1 = (double)body1.mass / _pow(1000., 3);
    double mass2 = (double)body2.mass / _pow(1000., 3);

    double3x3 Ia = InertiaTensor(pos1, mass1);
    double3x3 Ib = InertiaTensor(pos2, mass2);

    double3 dir = _normalize(pos2 - pos1);

    CollisionInfo info;
    info = CollisionResponce(0, mass1, mass2, Ia, Ib,
        pos1 + dir * (double)body1.radius,
        pos2 + -dir * (double)body2.radius,
        dir,
        (double3)body1.velocity * S_NORM,
        (double3)body2.velocity * S_NORM,
        (double3)body1.angular * 1000.,
        (double3)body2.angular * 1000.
    );

    info.velocity1 *= S_NORM_INV;
    info.velocity2 *= S_NORM_INV;
    info.angular1 *= 0.001;
    info.angular2 *= 0.001;

    if (isnan((float)info.velocity1.x)) { info.velocity1 = double3(0, 0, 0); }
    if (isnan((float)info.velocity2.x)) { info.velocity1 = double3(0, 0, 0); }

    return info;
}

//const Vector3 Planet::GetTidalAcceleration(Planet& planet)
//{
//    Vector3 p_relative = m_description.position - planet.GetPosition();
//
//    const long double radius = Vector3::Distance(Vector3::Zero, p_relative);
//    if (radius > 0)
//    {
//        const long double mass = m_description.mass;
//        const long double radius_m = radius * S_NORM * 1000;
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