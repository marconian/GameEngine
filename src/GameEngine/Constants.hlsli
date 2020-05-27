#pragma once

static const double G = 6.673e-11;
static const double S_NORM = 167420.;
static const double S_NORM_INV = 1. / S_NORM;
static const double EARTH_MASS = 5.97219e24;
static const double EARTH_DIAMETER = 16742;
static const double RATIO_SIZE_MASS = EARTH_DIAMETER / EARTH_MASS;


float scale(float value, float limit)
{
    return min(max(value, -limit), limit);
}