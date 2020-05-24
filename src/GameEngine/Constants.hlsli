#pragma once

static const double G = 6.673e-11;
static const double S_NORM = 167420.;


float scale(float value, float limit)
{
    return min(max(value, -limit), limit);
}