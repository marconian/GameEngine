#pragma once

#include "pch.h"
#include <typeinfo>
#include <initializer_list>
#include <vector>
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


const void split(std::string value, char seperator, std::vector<std::string>& values)
{
	std::string v = "";
	for (char c : value)
	{
		if (c != seperator) v += c;
		else
		{
			values.push_back(v);
			v = "";
		}
	}

	if (v != "") values.push_back(v);
}
