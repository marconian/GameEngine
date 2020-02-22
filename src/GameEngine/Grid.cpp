#include "pch.h"
#include "Grid.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Grid::Grid(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, bool msaa, int sampleCount) :
    m_device(device),
    m_backBufferFormat(backBufferFormat),
    m_depthBufferFormat(depthBufferFormat),
    m_msaa(msaa),
    m_sampleCount(sampleCount) 
{ 
    m_origin = Vector3::Zero;
    m_rotation = Quaternion::Identity;
    m_divisions = 20;
    m_cellsize = 2.f;

    CreateDeviceDependentResources();
}

void Grid::Render(ID3D12GraphicsCommandList* commandList)
{
    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
    m_effect->SetWorld(m_world);
    m_effect->Apply(commandList);

    m_batch->Begin(commandList);

    Vector3 xaxis(m_cellsize, 0.f, 0.f);
    Vector3 yaxis(0.f, 1.f, m_cellsize);

    for (size_t i = 0; i <= m_divisions; ++i)
    {
        float fPercent = float(i) / float(m_divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = xaxis * fPercent + m_origin;

        VertexPositionColor v1(scale - yaxis, Colors::White);
        VertexPositionColor v2(scale + yaxis, Colors::White);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= m_divisions; i++)
    {
        float fPercent = float(i) / float(m_divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + m_origin;

        VertexPositionColor v1(scale - xaxis, Colors::White);
        VertexPositionColor v2(scale + xaxis, Colors::White);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();
}

void Grid::Update() 
{

}

void Grid::CreateDeviceDependentResources()
{
    RenderTargetState rtState(m_backBufferFormat, m_depthBufferFormat);

    CD3DX12_RASTERIZER_DESC rastDesc;
    if (m_msaa)
    {
        rtState.sampleDesc.Count = m_sampleCount;
        rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
            D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
            D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
            0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
    }

    EffectPipelineStateDescription pdLine(
        &VertexPositionColor::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        rastDesc,
        rtState,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_device);
    m_effect = std::make_unique<BasicEffect>(m_device, EffectFlags::VertexColor, pdLine);
}

void Grid::CreateWindowSizeDependentResources(float height, float width) 
{

    m_world = Matrix::Identity;

    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
        Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        width / height, 0.1f, 10.f);
}