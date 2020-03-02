#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "Constants.h"
#include "Globals.h"
#include "Grid.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Grid::Grid() :
    //m_device(device),
    m_origin(Vector3::Zero),
    m_rotation(Quaternion::Identity),
    m_divisions(20),
    m_cellsize(2.f),
    m_color(Colors::White)
{ 
    CreateDeviceDependentResources();
}

void Grid::Render(ID3D12GraphicsCommandList* commandList)
{
    m_effect->SetWorld(g_world);
    m_effect->SetView(g_camera->View());
    m_effect->SetProjection(g_camera->Proj());
    m_effect->Apply(commandList);

    m_batch->Begin(commandList);

    Vector3 xaxis(m_cellsize, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, m_cellsize);

    for (size_t i = 0; i <= m_divisions; ++i)
    {
        float fPercent = float(i) / float(m_divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = xaxis * fPercent + m_origin;

        VertexPositionColor v1(scale - yaxis, m_color);
        VertexPositionColor v2(scale + yaxis, m_color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= m_divisions; i++)
    {
        float fPercent = float(i) / float(m_divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + m_origin;

        VertexPositionColor v1(scale - xaxis, m_color);
        VertexPositionColor v2(scale + xaxis, m_color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();
}

void Grid::Update(DX::StepTimer const& timer)
{

}

void Grid::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);

    CD3DX12_RASTERIZER_DESC rastDesc;
    if (MSAA_ENABLED)
    {
        rtState.sampleDesc.Count = SAMPLE_COUNT;
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

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device);
    m_effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pdLine);
}