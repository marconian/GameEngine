#pragma once

#include "pch.h"
#include "DeviceResources.h"
#include "Constants.h"
#include "Camera.h"
#include "Globals.h"

#include <map>
#include <vector>

using namespace DirectX::SimpleMath;

const std::unique_ptr<Camera> g_camera = std::make_unique<Camera>();

const Matrix g_world = Matrix::CreateWorld(Vector3::Zero, Vector3::Forward, Vector3::Up);

const std::unique_ptr<DX::DeviceResources> g_deviceResources = std::make_unique<DX::DeviceResources>(
    BACK_BUFFER_FORMAT,
    DEPTH_BUFFER_FORMAT,
    10,
    SAMPLE_COUNT,
    D3D_FEATURE_LEVEL_11_0,
    0,
    MSAA_ENABLED,
    SKY_COLOR
    );

std::vector<Planet> g_planets = std::vector<Planet>();

std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>> g_mvp_buffer;

const void CreateGlobalBuffers()
{
    g_mvp_buffer = std::make_unique<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>();
}

const void UpdateGlobalBuffers()
{
    // Update ModelViewProjection buffer
    Matrix _mv = XMMatrixMultiply(g_world, g_camera->View());
    Matrix _mp = XMMatrixMultiply(g_world, g_camera->Proj());
    Matrix _vp = XMMatrixMultiply(g_camera->View(), g_camera->Proj());
    Matrix _mvp = XMMatrixMultiply(_mv, g_camera->Proj());

    Buffers::ModelViewProjection mvp = {};
    mvp.m = XMMatrixTranspose(g_world);
    mvp.v = XMMatrixTranspose(g_camera->View());
    mvp.p = XMMatrixTranspose(g_camera->Proj());
    mvp.mv = XMMatrixTranspose(_mv);
    mvp.mp = XMMatrixTranspose(_mp);
    mvp.vp = XMMatrixTranspose(_vp);
    mvp.mvp = XMMatrixTranspose(_mvp);
    mvp.eye = g_camera->Position();
    g_mvp_buffer->Write(mvp);
}