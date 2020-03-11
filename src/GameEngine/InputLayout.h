#pragma once
class InputLayout
{
public:
	InputLayout(std::initializer_list<InputElement> elements);

	const D3D12_INPUT_LAYOUT_DESC GetLayout();

private:
	std::vector<InputElement> m_elements;

	const D3D12_INPUT_ELEMENT_DESC GetInputElementDesc(const InputElement& element, const unsigned int offset);
};