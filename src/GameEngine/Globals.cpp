#pragma once

#include "pch.h"
#include "DeviceResources.h"
#include "Constants.h"
#include "Camera.h"
#include "Globals.h"

using namespace DirectX::SimpleMath;

const std::unique_ptr<Camera> g_camera = std::make_unique<Camera>();

const Matrix g_world = Matrix::CreateWorld(Vector3::Zero, Vector3::Forward, Vector3::Up);

const std::unique_ptr<DX::DeviceResources> g_deviceResources = std::make_unique<DX::DeviceResources>(
    BACK_BUFFER_FORMAT,
    DEPTH_BUFFER_FORMAT, /* If we were only doing MSAA rendering, we could skip the non-MSAA depth/stencil buffer with DXGI_FORMAT_UNKNOWN */
    2,
    SAMPLE_COUNT,
    D3D_FEATURE_LEVEL_11_0,
    0,
    MSAA_ENABLED,
    SKY_COLOR
    );

std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>> g_mvp_buffer;

const void CreateGlobalBuffers()
{
    g_mvp_buffer = std::make_unique<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>(g_deviceResources->GetD3DDevice());
}

const void UpdateGlobalBuffers()
{
    // Update ModelViewProjection buffer
    Matrix _mv = XMMatrixMultiply(g_world, g_camera->View());
    Matrix _mvp = XMMatrixMultiply(_mv, g_camera->Proj());

    Buffers::ModelViewProjection mvp = {};
    mvp.mv = XMMatrixTranspose(_mv);
    mvp.mvp = XMMatrixTranspose(_mvp);
    g_mvp_buffer->Write(mvp);
}