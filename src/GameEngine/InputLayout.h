#pragma once
class InputLayout
{
public:
	InputLayout(std::initializer_list<InputElement> elements);

	D3D12_INPUT_LAYOUT_DESC GetLayout();

private:
	std::vector<InputElement> m_elements;

	static D3D12_INPUT_ELEMENT_DESC GetInputElementDesc(const InputElement& element, unsigned int offset);
};
