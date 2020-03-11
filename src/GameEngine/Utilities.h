#pragma once

const double rand(double min, double max);
const DirectX::SimpleMath::Vector3 randv(double min, double max);

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