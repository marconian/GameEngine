#pragma once

#include "DeviceResources.h"
#include "Camera.h"
#include "Buffers.h"
#include "Planet.h"

#include <vector>
#include <map>

using Microsoft::WRL::ComPtr;

extern const std::unique_ptr<DX::DeviceResources>									g_deviceResources;
extern const std::unique_ptr<Camera>												g_camera;
extern const DirectX::SimpleMath::Matrix											g_world;
extern std::vector<Planet>															g_planets;
extern unsigned int																	g_current;
extern std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>>		g_mvp_buffer;
extern float																		g_speed;

const void CreateGlobalBuffers();
const void UpdateGlobalBuffers();
const unsigned int CleanPlanets();
const unsigned int GetPlanetIndex(const int id);
Planet& GetPlanet(unsigned int id);