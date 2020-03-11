#include "pch.h"
#include "InputLayout.h"

InputLayout::InputLayout(std::initializer_list<InputElement> elements) :
    m_elements(elements) { }

const D3D12_INPUT_LAYOUT_DESC InputLayout::GetLayout()
{
    D3D12_INPUT_LAYOUT_DESC inputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC>* elements = new std::vector<D3D12_INPUT_ELEMENT_DESC>();

    unsigned int slot = 0;
    unsigned int offset = 0;

    for (InputElement& element : m_elements)
    {
        if (element.slot != slot)
        {
            offset = 0;
            slot = element.slot;
        }

        auto desc = GetInputElementDesc(element, offset);
        elements->emplace_back(desc);

        offset += element.size;
    }

    inputLayout.NumElements = elements->size();
    inputLayout.pInputElementDescs = elements->data();

    return inputLayout;
}

const D3D12_INPUT_ELEMENT_DESC InputLayout::GetInputElementDesc(const InputElement& element, const unsigned int offset)
{
    DXGI_FORMAT format;
    D3D12_INPUT_CLASSIFICATION classification;
    unsigned int stepRate;

    if (element.type == typeid(int).name())
    {
        format = DXGI_FORMAT_R32_UINT;
    }
    else if (element.type == typeid(float).name())
    {
        format = DXGI_FORMAT_R32_FLOAT;
    }
    else if (element.type == typeid(XMFLOAT2).name() || element.type == typeid(Vector2).name())
    {
        format = DXGI_FORMAT_R32G32_FLOAT;
    }
    else if (element.type == typeid(XMFLOAT3).name() || element.type == typeid(Vector3).name())
    {
        format = DXGI_FORMAT_R32G32B32_FLOAT;
    }
    else if (element.type == typeid(XMFLOAT4).name() || element.type == typeid(Vector4).name())
    {
        format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
    else throw std::invalid_argument("Unsupported type.");

    switch (element.mode)
    {
    case Vertex:
        classification = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        stepRate = 0;
        break;
    case Instance:
        classification = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        stepRate = 1;
        break;
    default:
        throw std::invalid_argument("Unsupported input classification.");
    }

    return D3D12_INPUT_ELEMENT_DESC{ element.name.c_str(), 0, format, element.slot, offset, classification, stepRate };
}