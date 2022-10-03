#include "pch.h"
#pragma once

#include "StepTimer.h"
#include "Constants.h"
#include "Globals.h"
#include "Cluster.h"
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

	std::vector<const Planet*> planets{};

	float xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
	for (auto& planet : g_planets)
	{
		const auto position = planet.GetPosition();
		if (position.x < xmin) xmin = position.x;
		if (position.y < ymin) ymin = position.y;
		if (position.z < zmin) zmin = position.z;
		if (position.x > xmax) xmax = position.x;
		if (position.y > ymax) ymax = position.y;
		if (position.z > zmax) zmax = position.z;
		planets.push_back(&planet);
	}

	AddBlock(xmin, ymin, zmin, xmax, ymax, zmax);

	const auto& cluster = Cluster({ xmin, ymin, zmin, xmax, ymax, zmax}, planets, 8);
	
	for (auto& line : cluster.GetGridLines(m_color))
		m_lines.push_back(line);
}

void Grid::AddBlock(const float xmin, const float ymin, const float zmin, const float xmax, const float ymax, const float zmax)
{
	m_lines.push_back(Line(
		{{xmin, ymin, zmin}, m_color},
		{{xmin, ymax, zmin}, m_color}
	));
	m_lines.push_back(Line(
		{{xmin, ymax, zmin}, m_color},
		{{xmax, ymax, zmin}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymax, zmin}, m_color},
		{{xmax, ymin, zmin}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymin, zmin}, m_color},
		{{xmin, ymin, zmin}, m_color}
	));

	m_lines.push_back(Line(
		{{xmin, ymin, zmax}, m_color},
		{{xmin, ymax, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmin, ymax, zmax}, m_color},
		{{xmax, ymax, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymax, zmax}, m_color},
		{{xmax, ymin, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymin, zmax}, m_color},
		{{xmin, ymin, zmax}, m_color}
	));

	m_lines.push_back(Line(
		{{xmin, ymin, zmin}, m_color},
		{{xmin, ymin, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmin, ymax, zmin}, m_color},
		{{xmin, ymax, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymax, zmin}, m_color},
		{{xmax, ymax, zmax}, m_color}
	));
	m_lines.push_back(Line(
		{{xmax, ymin, zmin}, m_color},
		{{xmax, ymin, zmax}, m_color}
	));
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
