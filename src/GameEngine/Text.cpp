#pragma once

#include "pch.h"
#include "StepTimer.h"
#include "Constants.h"
#include "Globals.h"
#include "FontTools.h"
#include "Text.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using Microsoft::WRL::ComPtr;

Text::Text()
{
	CreateDeviceDependentResources();
}

void Text::Print(std::string text, Vector2 position, Align align, XMVECTORF32 color)
{
	ID3D12GraphicsCommandList* commandList = g_device_resources->GetCommandList();

	ID3D12DescriptorHeap* heaps[] = {m_resourceDescriptors->Heap()};
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	m_batch->Begin(commandList);

	const Vector2 size = m_font->MeasureString(text.c_str());

	Vector2 origin;
	switch (align)
	{
	case Left:
		origin = Vector2::Zero;
		break;
	case Right:
		origin = size;
		break;
	case Center:
		origin = size / 2.f;
		break;
	default:
		origin = Vector2::Zero;
		break;
	}

	m_font->DrawString(m_batch.get(), text.c_str(),
	                   position, color, 0.f, origin);

	m_batch->End();
}

void Text::CreateDeviceDependentResources()
{
	auto device = g_device_resources->GetD3DDevice();

	m_resourceDescriptors = std::make_unique<DescriptorHeap>(device,
	                                                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	                                                         D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	                                                         2);

	ResourceUploadBatch resourceUpload(device);

	resourceUpload.Begin();

	m_font = std::make_unique<SpriteFont>(device, resourceUpload,
	                                      GetFontsPath("quicksand", 12).c_str(),
	                                      m_resourceDescriptors->GetCpuHandle(0),
	                                      m_resourceDescriptors->GetGpuHandle(0));

	RenderTargetState rtState(BACK_BUFFER_FORMAT, DEPTH_BUFFER_FORMAT);

	CD3DX12_RASTERIZER_DESC rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
	                                                           D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
	                                                           D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
	                                                           0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	SpriteBatchPipelineStateDescription pd(rtState,
	                                       &CommonStates::AlphaBlend,
	                                       &CommonStates::DepthDefault,
	                                       &rastDesc,
	                                       nullptr);

	m_batch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);

	auto uploadResourcesFinished = resourceUpload.End(g_device_resources->GetCommandQueue());

	uploadResourcesFinished.wait();
}

void Text::SetViewport(const float width, const float height)
{
	D3D12_VIEWPORT viewport = {
		0.0f, 0.0f,
		width, height,
		D3D12_MIN_DEPTH, D3D12_MAX_DEPTH
	};

	m_batch->SetViewport(viewport);
}
