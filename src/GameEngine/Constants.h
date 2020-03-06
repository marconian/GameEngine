#pragma once

constexpr double G = 6.673e-11; // 6.673e-11;
constexpr double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;

constexpr double SUN_MASS = 1.989e30;
constexpr double SUN_DIAMETER = 1392680;

constexpr double EARTH_MASS = 5.97219e24;
constexpr double EARTH_DIAMETER = 16742;
constexpr double EARTH_SUN_DIST = 149600000;
constexpr double EARTH_SUN_VELOCITY = 107208;

constexpr double MOON_MASS = 7.34767309e22;
constexpr double MOON_DIAMETER = 3474.2;
constexpr double MOON_EARTH_DIST = 384400;
constexpr double MOON_EARTH_VELOCITY = 1022;

constexpr double M_NORM = EARTH_MASS;
constexpr double S_NORM = EARTH_DIAMETER;
const double G_NORMALIZED = sqrt(G * pow(S_NORM, 3));

constexpr double TIME_DELTA = 1;
constexpr double DEFAULT_ZOOM = 1;
constexpr double ROTATION_GAIN = 1;
constexpr double ROTATION_GAIN_MOUSE = 0.004;
constexpr double MOVEMENT_GAIN = 0.1;

constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;

constexpr bool MSAA_ENABLED = true;
constexpr int SAMPLE_COUNT = 4;

constexpr DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f };