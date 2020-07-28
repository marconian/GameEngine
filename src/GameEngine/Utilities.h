#pragma once

#include <optional>

const double rand(double const min, double const max);
const DirectX::SimpleMath::Vector3 randv(double const min, double const max);

template<typename T>
const T& rand(std::vector<T> const& data)
{
	uint32_t const index = static_cast<uint32_t>(ceil(rand(-1, data.size() - 1)));
	return data[index];
}

const void split(std::string value, char seperator, std::vector<std::string>& values);

const std::vector<double> euler(std::function<double(double v, double t, int32_t i)> const& _action, std::function<bool(double v, double t, int32_t i)> const& _while, std::optional<double> const initial, double const step = 1.);

template<class T, size_t S>
const void normalize(std::array<T, S>& values)
{
	T sum = 0;
	for (int i = 0; i < S; i++)
		sum += values[i];

	T diff = 0;
	if (sum > 0)
	{
		sum = 1 / sum;
		for (int i = 0; i < S; i++)
		{
			values[i] *= sum;
			diff += values[i];
		}

		diff = 1 - diff;
		if (diff != 0)
		{
			for (int i = 0; i < S; i++)
			{
				values[i] += diff;

				if (values[i] < 0)
				{
					diff = values[i];
					values[i] = 0;
				}
				else break;
			}
		}
	}
}

enum InputElementType
{
	Vertex,
	Instance
};

struct InputElement
{
	template<class T>
	InputElement(const char* name, const unsigned int slot, const InputElementType mode, const T type) :
		name(std::string(name)),
		slot(slot),
		mode(mode),
		type(std::string(typeid(type).name())),
		size(sizeof(type)) {}

	std::string name;
	unsigned int slot;
	InputElementType mode;
	const std::string type;
	const size_t size;
};