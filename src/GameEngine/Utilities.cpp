#pragma once

#include "pch.h"
#include <typeinfo>
#include <initializer_list>
#include "Utilities.h"

const double rand(const double min, const double max)
{
    const std::pair<int, int> values = std::minmax({ rand(), rand() });
    const double upperLimit = values.second;
    const double value = values.first;

    return (value / upperLimit) * (max - min) + min;
}

const DirectX::SimpleMath::Vector3 randv(const double min, const double max)
{
    return DirectX::SimpleMath::Vector3(
        rand(min, max),
        rand(min, max),
        rand(min, max)
    );
}
