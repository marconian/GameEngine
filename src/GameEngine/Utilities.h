#pragma once

#include <optional>

double rand(double min, double max);
float rand(float min, float max);
DirectX::SimpleMath::Vector3 randv(double min, double max);

template <typename T>
const T& rand(std::vector<T> const& data)
{
	uint32_t const index = static_cast<uint32_t>(ceil(rand(-1, data.size() - 1)));
	return data[index];
}

void split(const std::string& value, char seperator, std::vector<std::string>& values);

std::vector<double> euler(std::function<double(double v, double t, int32_t i)> const& _action,
                          std::function<bool(double v, double t, int32_t i)> const& _while,
                          std::optional<double> initial, double step = 1.);

template <class T, size_t S>
void normalize(std::array<T, S>& values)
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
	template <class T>
	InputElement(const char* name, const unsigned int slot, const InputElementType mode, const T type) :
		name(std::string(name)),
		slot(slot),
		mode(mode),
		type(std::string(typeid(type).name())),
		size(sizeof(type))
	{
	}

	std::string name;
	unsigned int slot;
	InputElementType mode;
	const std::string type;
	const size_t size;
};
