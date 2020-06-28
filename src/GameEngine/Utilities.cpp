#pragma once

#include "pch.h"
#include <typeinfo>
#include <initializer_list>
#include <vector>
#include <optional>
#include "Utilities.h"

const double rand(double const min, double const max)
{
    const std::pair<int, int> values = std::minmax({ rand(), rand() });
    const double upperLimit = values.second;
    const double value = values.first;

    return (value / upperLimit) * (max - min) + min;
}

const DirectX::SimpleMath::Vector3 randv(double const min, double const max)
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

const std::vector<double> euler(std::function<double(double v, double t, int32_t i)> const& _action, std::function<bool(double v, double t, int32_t i)> const& _while, std::optional<double> const initial, double const step)
{
	std::vector<double> values = {};

	double value = 0;
	int32_t i = 0;

	if (initial.has_value())
	{
		value = initial.value();
		i++;
		//
		//values.push_back(value);
	}

	while (_while(value, i * step, i))
	{
		double d = _action(value, i * step, i) * step;
		if (isnan(d)) d = 0;

		value += d;
		i++;

		values.push_back(value);
	}

	return values;
}