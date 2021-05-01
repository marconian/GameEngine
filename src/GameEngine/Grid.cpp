#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "Constants.h"
#include "Globals.h"
#include "Grid.h"

#include <iostream>
#include <memory>
#include <vector>
#include <array>

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using Microsoft::WRL::ComPtr;

Grid::Grid() :
	m_origin(Vector3::Zero),
	m_rotation(Quaternion::Identity),
	m_size(20),
	m_cellsize(2.f),
	m_color(Colors::White),
	m_lines{}
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

	for (Line& line : m_lines)
		m_batch->DrawLine(line.p1, line.p2);

	m_batch->End();
}

void Grid::Update(DX::StepTimer const& timer)
{
	m_lines.clear();
	m_lines.shrink_to_fit();

	size_t cells = static_cast<size_t>(floor(m_size / m_cellsize));
	cells += cells % 2;

	float size = cells * m_cellsize;
	float distance = size / 2.f;

	int start = cells / 2;
	for (int i = -start; i <= start; i++)
	{
		float offset = i * m_cellsize + m_cellsize * .5;

		VertexPositionColor x1({m_origin.x - distance, m_origin.y, m_origin.z + offset}, m_color);
		VertexPositionColor x2({m_origin.x + distance, m_origin.y, m_origin.z + offset}, m_color);

		VertexPositionColor y1({m_origin.x + offset, m_origin.y, m_origin.z - distance}, m_color);
		VertexPositionColor y2({m_origin.x + offset, m_origin.y, m_origin.z + distance}, m_color);

		m_lines.push_back(Line(x1, x2));
		m_lines.push_back(Line(y1, y2));
	}
}

void Grid::CreateDeviceDependentResources()
{
	auto device = g_device_resources->GetD3DDevice();

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
