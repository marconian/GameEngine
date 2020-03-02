#pragma once

#include "DeviceResources.h"
#include "Camera.h"
#include "Buffers.h"

using Microsoft::WRL::ComPtr;

extern const std::unique_ptr<DX::DeviceResources>						g_deviceResources;
extern const std::unique_ptr<Camera>									g_camera;
extern const DirectX::SimpleMath::Matrix								g_world;
extern std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>      g_mvp_buffer;

const void CreateGlobalBuffers();
const void UpdateGlobalBuffers();