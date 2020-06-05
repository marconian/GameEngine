#pragma once

const double rand(const double min, const double max);
const DirectX::SimpleMath::Vector3 randv(const double min, const double max);

template<typename T>
const T& rand(const std::vector<T>& data)
{
	const UINT index = rand(0, data.size() - 1);
	return data[index];
}

const void split(std::string value, char seperator, std::vector<std::string>& values);

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