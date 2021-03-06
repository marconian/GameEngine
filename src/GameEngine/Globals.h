#pragma once

#include "DeviceResources.h"
#include "Camera.h"
#include "Buffers.h"
#include "Planet.h"

#include <vector>
#include <map>

using Microsoft::WRL::ComPtr;

extern const std::unique_ptr<DX::DeviceResources> g_device_resources;
extern const std::unique_ptr<Camera> g_camera;
extern const DirectX::SimpleMath::Matrix g_world;
extern std::vector<Planet> g_planets;
extern std::map<uint32_t, Composition<float>> g_compositions;
extern std::map<uint32_t, std::vector<DepthInfo>> g_profiles;
extern std::map<std::string, std::vector<Planet*>> g_quadrants;
extern unsigned int g_current;
extern unsigned int g_quadrantSize;
extern unsigned int g_collisions;
extern std::unique_ptr<Buffers::ConstantBuffer<Buffers::Settings>> g_settings_buffer;
extern std::unique_ptr<Buffers::ConstantBuffer<Buffers::ModelViewProjection>> g_mvp_buffer;
extern float g_speed;
extern bool g_coreView;

void CreateGlobalBuffers();
void UpdateGlobalBuffers();
unsigned int CleanPlanets();
unsigned int GetPlanetIndex(int id);
Planet& GetPlanet(unsigned int id);
