#pragma once

#include "pch.h"
#include <typeinfo>
#include <initializer_list>
#include "Utilities.h"

const double rand(double min, double max)
{
    std::pair<int, int> values = std::minmax({ rand(), rand() });
    double upperLimit = values.second;
    double value = values.first;

    return (value / upperLimit) * max + min;
}

const DirectX::SimpleMath::Vector3 randv(double min, double max)
{
    return DirectX::SimpleMath::Vector3(
        rand(min, max),
        rand(min, max),
        rand(min, max)
    );
}