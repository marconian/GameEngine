#pragma once

constexpr double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;
const double PI_RAD = PI / 180.;

constexpr double G = 6.673e-11;
constexpr double c = 2.99792458e8; // speed of light (ms-1)
constexpr double h = 6.62607e-34; // Planck's constant (Js)
constexpr double eV = 1.60218e-19; // electron volt(J)
constexpr double Rh = 2.179e-18; // Rydberg constant for H (J)

constexpr double SUN_MASS = 1.989e30;
constexpr double SUN_DIAMETER = 1392680;

const double SYSTEM_MASS = SUN_MASS * 1.0014;

constexpr double EARTH_MASS = 5.97219e24;
constexpr double EARTH_DIAMETER = 16742;
constexpr double EARTH_SUN_DIST = 149600000;
constexpr double EARTH_SUN_VELOCITY = 107208;

constexpr double MOON_MASS = 7.34767309e22;
constexpr double MOON_DIAMETER = 3474.2;
constexpr double MOON_EARTH_DIST = 384400;
constexpr double MOON_EARTH_VELOCITY = 1022;

constexpr double S_NORM = EARTH_DIAMETER * 10.;
const double G_NORMALIZED = sqrt(G * pow(S_NORM, 3));

const int SECTOR_SIZE = SUN_DIAMETER / S_NORM;
const int SECTOR_DIMS = (EARTH_SUN_DIST / S_NORM) * 3 / SECTOR_SIZE;

constexpr double TIME_DELTA = 1000;
constexpr double DEFAULT_ZOOM = 1;
constexpr double ROTATION_GAIN = 1;
constexpr double ROTATION_GAIN_MOUSE = 0.004;
constexpr double MOVEMENT_GAIN = 0.1;

constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;

constexpr bool MSAA_ENABLED = true;
constexpr int SAMPLE_COUNT = 4;

constexpr DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f };