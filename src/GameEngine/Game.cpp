//
// Game.cpp
//

#pragma once

#include "pch.h"
#include "Constants.h"
#include "Globals.h"
#include "Game.h"

#include <iostream>
#include <memory>

extern void ExitGame();

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace
{
    //const DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f};
    const DirectX::XMVECTORF32 GRID_COLOR = Colors::DarkGray;
    const DirectX::XMVECTORF32 TERRAIN_COLOR = Colors::SaddleBrown;
}

Game::Game() noexcept(false) :
    m_show_grid(false),
    m_elapsed(0),
    m_position(Vector3::Zero),
    m_zoom(DEFAULT_ZOOM),
    m_pitch(60),
    m_yaw(0),
    m_changing_planet(false)
{
    g_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (g_deviceResources)
        g_deviceResources->WaitForGpu();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();

    m_mouse->SetWindow(window);

    g_deviceResources->SetWindow(window, width, height);

    g_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    g_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    
    //m_timer.SetFixedTimeStep(true);
    //m_timer.SetTargetElapsedSeconds(1.0 / 60);
    
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    m_planetRenderer->Update(timer);

    if (m_show_grid)
        m_graphic_grid->Update(timer);

    auto mouse = m_mouse->GetState();
    m_mouseButtons.Update(mouse);

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    bool cleanedPlanets = false;
    //unsigned int idx = g_current;

    // Draw the scene.
    Planet* planet = &g_planets[g_current];

    m_elapsed += elapsedTime;
    if (m_elapsed > 10) {
        m_elapsed = 0;

        g_current = CleanPlanets();
        cleanedPlanets = true;
    }

    // Execute input actions

    //if (mouse.positionMode == Mouse::MODE_RELATIVE) { }
    //if (mouse.rightButton)
    //    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    //else
    //    m_mouse->SetMode(Mouse::MODE_ABSOLUTE);

    const bool keyEnter = m_keyboardButtons.IsKeyPressed(m_keyboard->Enter);
    const bool keySpace = m_keyboardButtons.IsKeyPressed(m_keyboard->Space);
    const bool keyPeriod = m_keyboardButtons.IsKeyPressed(m_keyboard->OemPeriod);
    const bool keyComma = m_keyboardButtons.IsKeyPressed(m_keyboard->OemComma);
    const bool keyTab = m_keyboardButtons.IsKeyPressed(m_keyboard->Tab);
    const bool keyEscape = m_keyboardButtons.IsKeyPressed(m_keyboard->Escape);
    const bool keyC = m_keyboardButtons.IsKeyPressed(m_keyboard->C);

    if (keySpace)
        m_show_grid = !m_show_grid;

    if (kb.RightAlt && keyEnter)
        g_deviceResources->ToggleFullScreen();

    if (keyPeriod || keyComma)
    {
        float speed = g_speed;
        if (keyPeriod) speed *= 10.;
        else if (keyComma) speed *= .1;

        if (speed < 1) speed = 1.;
        else if (speed > 100000) speed = 100000.;

        g_speed = speed;
    }

    if ((keyTab || keyEscape || keyC) && g_planets.size())
    {
        if (keyTab)
            g_current = (g_current + (unsigned int)1) % (unsigned int)g_planets.size();
        else if (keyC)
        {
            for (int i = g_current; i < g_planets.size(); i++)
            {
                if (g_planets[i].collisions > 0 && g_planets[i].id != g_planets[g_current].id)
                {
                    g_current = i;
                    break;
                }
            }
        }
        else g_current = 0;

        if (!cleanedPlanets)
        {
            g_current = CleanPlanets();
            cleanedPlanets = true;
        }
    }

    if (g_planets[g_current].id != planet->id)
    {
        m_changing_planet = true;

        planet = &g_planets[g_current];

        m_zoom = DEFAULT_ZOOM;
        m_pitch, m_yaw = 0;

        m_planetRenderer->Refresh();
    }

    if (!m_changing_planet) // Block this controls on planet change execution
    {
        if (kb.PageUp || kb.Subtract || kb.OemMinus)
            m_zoom += (m_zoom * 0.1) * MOVEMENT_GAIN;
        if (kb.PageDown || kb.Add || kb.OemPlus)
            m_zoom -= (m_zoom * 0.1) * MOVEMENT_GAIN;

        float move_pitch = 0.f;
        if (kb.Up || kb.W)
            move_pitch = 2.f;
        if (kb.Down || kb.S)
            move_pitch = -2.f;
        move_pitch *= ROTATION_GAIN;

        float move_yaw = 0.f;
        if (kb.Left || kb.A)
            move_yaw = 2.f;
        if (kb.Right || kb.D)
            move_yaw = -2.f;
        move_yaw *= ROTATION_GAIN;

        if (m_zoom < 0)
            m_zoom = 0;

        m_yaw = fmod(m_yaw + move_yaw, 359);
        m_pitch = m_pitch + move_pitch;

        m_pitch = std::max(-90.f, m_pitch);
        m_pitch = std::min(90.f, m_pitch);
    }

    Vector3 moveTo = GetRelativePosition();
    float distance = Vector3::Distance(m_position, moveTo);
    m_position += Vector3::Lerp(Vector3::Zero, moveTo - m_position, cos(distance * elapsedTime));

    if (isnan(m_position.x + m_position.y + m_position.z))
    {
        m_changing_planet = false;
    }

    if (m_changing_planet && distance < planet->GetScreenSize() * 20.)
        m_changing_planet = false;

    g_camera->Position(m_position);
    g_camera->Target(planet->GetPosition());

    //if (m_mouseButtons.leftButton == m_mouseButtons.PRESSED)
    //{
    //    auto windowSize = g_deviceResources->GetOutputSize();
    //    double windowWidth = static_cast<double>(windowSize.right - windowSize.left);
    //    double windowHeight = static_cast<double>(windowSize.bottom - windowSize.top);
    //
    //    Vector3 target;
    //    (-(m_position - planet.GetPosition())).Normalize(target);
    //
    //    auto screencoords = Vector2((mouse.x / windowWidth) - .5, (mouse.y / windowHeight) - .5);
    //    auto speed = static_cast<float>(((rand() % 1500000)) / S_NORM);
    //
    //    target.z += screencoords.x;
    //    target.y += screencoords.y;
    //
    //    target *= speed;
    //
    //    CreatePlanetAtPosition(m_position, target);
    //}

    UpdateGlobalBuffers();

    PIXEndEvent();
}
#pragma endregion

void Game::CreateSolarSystem()
{
    const UINT noOfPlanets = 1000;
    double totalMass = 0;
    std::vector<double> masses;
    for (int i = 0; i < noOfPlanets; i++)
    {
        const double mass = rand(MOON_MASS / 10., EARTH_MASS * 100.);
        totalMass += mass;
        masses.push_back(rand(MOON_MASS / 10., EARTH_MASS * 100.));
    }

    CreatePlanet(SUN_MASS - totalMass, pow(SUN_MASS - totalMass, 1 / 3.) / DENSITY_NORM, TERRAIN_COLOR, Vector3::Zero, Vector3::Zero, 0);
    //CreatePlanet(SUN_MASS, SUN_DIAMETER * .5, TERRAIN_COLOR, Vector3::Zero, Vector3::Zero, 0);

    CreatePlanet(EARTH_MASS, pow(EARTH_MASS, 1 / 3.) / DENSITY_NORM, Colors::ForestGreen,
        { EARTH_SUN_DIST, 0, 0 },
        { 0, 0, 1 },
        EARTH_SUN_VELOCITY);

    CreatePlanet(MOON_MASS, pow(MOON_MASS, 1 / 3.) / DENSITY_NORM, Colors::GhostWhite,
        { EARTH_SUN_DIST, 0, MOON_EARTH_DIST },
        { 0, 0, 1 },
        EARTH_SUN_VELOCITY + MOON_EARTH_VELOCITY);

    const double maxDistance = SUN_DIAMETER * 2.;
    const auto colors = std::vector({
        Colors::LightSlateGray,
        Colors::Peru,
        Colors::Azure,
        Colors::DeepSkyBlue,
        Colors::Honeydew,
        Colors::Aquamarine
        });

    for (int i = 0; i < noOfPlanets; i++)
    {
        const double mass = masses[i];
        const double radius = pow(mass, 1 / 3.) / DENSITY_NORM;
        const double velocity = rand(EARTH_SUN_VELOCITY * .1, EARTH_SUN_VELOCITY * 10.);

        double distance = rand(-maxDistance, maxDistance);
        while (distance > -SUN_DIAMETER * 1.5 && distance < SUN_DIAMETER * 1.5)
            distance = rand(-maxDistance, maxDistance);

        Vector3 position = randv(-1, 1);
        position = XMVector3Transform(position,
            XMMatrixRotationAxis(Vector3::UnitY, XMConvertToRadians(rand(0, 359)))) * distance;
        position.y = rand(-SUN_DIAMETER / 4., SUN_DIAMETER / 4.);

        Vector3 direction;
        (-position).Normalize(direction);
        direction = Vector3(-direction.z, direction.y, direction.x);
        direction.y = rand(-.05, .05);

        CreatePlanet(mass, radius, rand(colors), position, direction, velocity);
    }
}

Planet& Game::CreatePlanet(double mass, double radius, XMVECTORF32 color, Vector3 position, Vector3 direction, float velocity)
{
    Planet p(mass, radius, position / S_NORM, direction, (velocity / 3600) / S_NORM, color);

    g_planets.push_back(p);

    return p;
}

Vector3 Game::GetRelativePosition()
{
    Planet& planet = g_planets[g_current];

    float pitch = m_pitch * PI_RAD;
    float yaw = m_yaw * PI_RAD;

    float radius = planet.GetScreenSize() * 2 * m_zoom;

    float step_x = cos(yaw) * radius;
    float step_z = sin(yaw) * radius;
    float step_y = sin(pitch) * radius;

    return planet.GetPosition() + Vector3(step_x, step_y, step_z);
}

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0) return;

    // Prepare the command list to render a new frame.
    g_deviceResources->Prepare(D3D12_RESOURCE_STATE_PRESENT);
    auto commandList = g_deviceResources->GetCommandList();

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    ClearBackBuffers();
    SetRenderTargetMsaa();
    RenderMain();
    ResolveRenderTargetMsaa();
    SetRenderTargetMain();
    RenderInterface();

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(g_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");

    g_deviceResources->Present();
    m_graphicsMemory->Commit(g_deviceResources->GetCommandQueue());
    g_deviceResources->WaitForGpu();

    PIXEndEvent(g_deviceResources->GetCommandQueue());
}

void Game::RenderMain()
{
    auto commandList = g_deviceResources->GetCommandList();

    if (m_show_grid) m_graphic_grid->Render(commandList);
    m_planetRenderer->Render(commandList);
}

void Game::RenderInterface()
{
    int collisions = 0;
    for (int i = 1; i < g_planets.size(); i++)
        collisions += g_planets[i].collisions;
    m_interface->Print("Collisions: " + std::to_string(collisions), Vector2(10, 10), Align::Left, Colors::Azure);
}

// Helper method to clear the back buffers.
void Game::ClearBackBuffers()
{
    auto commandList = g_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor, dsvDescriptor;
    if (MSAA_ENABLED)
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            g_deviceResources->GetRenderTargetMsaa(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->ResourceBarrier(1, &barrier);
    }

    rtvDescriptor = g_deviceResources->GetRenderTargetViewMsaa();
    dsvDescriptor = g_deviceResources->GetDepthStencilViewMsaa();

    commandList->ClearRenderTargetView(g_deviceResources->GetRenderTargetView(), SKY_COLOR, 0, nullptr);
    commandList->ClearDepthStencilView(g_deviceResources->GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList->ClearRenderTargetView(g_deviceResources->GetRenderTargetViewMsaa(), SKY_COLOR, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    commandList->RSSetViewports(1, &g_deviceResources->GetScreenViewport());
    commandList->RSSetScissorRects(1, &g_deviceResources->GetScissorRect());

    PIXEndEvent(commandList);
}

void Game::SetRenderTargetMain()
{
    auto commandList = g_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Set Render Target Main");

    commandList->OMSetRenderTargets(1, 
        &g_deviceResources->GetRenderTargetView(), FALSE,
        &g_deviceResources->GetDepthStencilView());

    PIXEndEvent(commandList);
}
void Game::SetRenderTargetMsaa()
{
    auto commandList = g_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Set Render Target Msaa");

    commandList->OMSetRenderTargets(1,
        &g_deviceResources->GetRenderTargetViewMsaa(), FALSE,
        &g_deviceResources->GetDepthStencilViewMsaa());

    PIXEndEvent(commandList);
}

void Game::ResolveRenderTargetMsaa()
{
    auto commandList = g_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Resolve Render Target Msaa");

    auto backBuffer = g_deviceResources->GetRenderTarget();
    auto backBufferMsaa = g_deviceResources->GetRenderTargetMsaa();

    D3D12_RESOURCE_BARRIER barriers[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(
            backBufferMsaa,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_DEST)
    };

    commandList->ResourceBarrier(2, barriers);

    commandList->ResolveSubresource(backBuffer, 0, backBufferMsaa, 0, BACK_BUFFER_FORMAT);

    // Set render target for UI which is typically rendered without MSAA.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer,
        D3D12_RESOURCE_STATE_RESOLVE_DEST,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    PIXEndEvent(commandList);
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_keyboardButtons.Reset();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = g_deviceResources->GetOutputSize();
    g_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!g_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = g_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    CreateGlobalBuffers();
    CreateSolarSystem();

    CleanPlanets();

    m_planetRenderer = std::make_unique<PlanetRenderer>();

    // Setup grid.
    m_graphic_grid = std::make_unique<Grid>();
    m_graphic_grid->SetColor(GRID_COLOR);

    float grid_size = EARTH_SUN_DIST * 2 / S_NORM;
    int grid_divisions = round(grid_size / (SUN_DIAMETER * 2 / S_NORM));

    m_graphic_grid->SetOrigin({ 0, 0, 0 });
    m_graphic_grid->SetDivisionsAndSize(grid_divisions, grid_size);

    m_interface = std::make_unique<Text>();
    m_interface->CreateDeviceDependentResources();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto outputSize = g_deviceResources->GetOutputSize();
    int backBufferWidth = std::max<int>(static_cast<int>(outputSize.right - outputSize.left), 1u);
    int backBufferHeight = std::max<int>(static_cast<int>(outputSize.bottom - outputSize.top), 1u);

    if (backBufferWidth == 1 || backBufferHeight == 1) {
        GetDefaultSize(backBufferWidth, backBufferHeight);
    }

    m_interface->SetViewport(backBufferWidth, backBufferHeight);

    g_camera->InitProjMatrix(XM_PI / 4.f, backBufferWidth, backBufferHeight, 0.01f, 10000.f);
}

void Game::OnDeviceLost()
{
    m_graphic_grid.reset();
    m_interface.reset();
    g_planets.clear();

    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
