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
    m_position(Vector3::Zero),
    m_zoom(DEFAULT_ZOOM),
    m_pitch(0),
    m_yaw(0),
    m_current(0),
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

    auto mouse = m_mouse->GetState();
    m_mouseButtons.Update(mouse);

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    // Draw the scene.
    m_planetRenderer->Update(timer);

    if (m_show_grid)
        m_graphic_grid->Update(timer);

    // Execute input actions

    //if (mouse.positionMode == Mouse::MODE_RELATIVE) { }
    //if (mouse.rightButton)
    //    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    //else
    //    m_mouse->SetMode(Mouse::MODE_ABSOLUTE);

    if (m_keyboardButtons.IsKeyPressed(m_keyboard->Tab) && g_planets.size())
    {
        m_current = (m_current + 1) % g_planets.size();
        m_zoom = DEFAULT_ZOOM;
        m_pitch, m_yaw = 0;

        m_changing_planet = true;
    }

    Planet& planet = g_planets[m_current];

    if (m_keyboardButtons.IsKeyPressed(m_keyboard->Space))
        m_show_grid = !m_show_grid;

    if (!m_changing_planet)
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

    if (m_changing_planet && distance < planet.GetSize() * 10.)
        m_changing_planet = false;

    g_camera->Position(m_position);
    g_camera->Target(planet.GetPosition());


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

Vector3 Game::GetRelativePosition()
{
    Planet& planet = g_planets[m_current];

    float pitch = m_pitch * PI_RAD;
    float yaw = m_yaw * PI_RAD;

    float radius = planet.GetSize() * 2 * m_zoom;

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
    if (m_timer.GetFrameCount() == 0)
        return;

    // Prepare the command list to render a new frame.
    g_deviceResources->Prepare(MSAA_ENABLED ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);
    Clear();

    auto commandList = g_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    if (m_show_grid) m_graphic_grid->Render(commandList);
    m_planetRenderer->Render(commandList);

    PIXEndEvent(commandList);

    if (MSAA_ENABLED)
    {
        // Resolve the MSAA render target.
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Resolve");

        auto backBuffer = g_deviceResources->GetRenderTarget();
        auto backBufferMsaa = g_deviceResources->GetRenderTargetMsaa();

        {
            D3D12_RESOURCE_BARRIER barriers[2] =
            {
                CD3DX12_RESOURCE_BARRIER::Transition(
                    backBufferMsaa,
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(
                    backBuffer,
                    D3D12_RESOURCE_STATE_PRESENT,
                    D3D12_RESOURCE_STATE_RESOLVE_DEST)
            };

            commandList->ResourceBarrier(2, barriers);
        }

        commandList->ResolveSubresource(backBuffer, 0, backBufferMsaa, 0, BACK_BUFFER_FORMAT);

        PIXEndEvent(commandList);

        // Set render target for UI which is typically rendered without MSAA.
        {
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                backBuffer,
                D3D12_RESOURCE_STATE_RESOLVE_DEST,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList->ResourceBarrier(1, &barrier);
        }
    }

    // Show the new frame.
    PIXBeginEvent(g_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    g_deviceResources->Present();
    m_graphicsMemory->Commit(g_deviceResources->GetCommandQueue());
    PIXEndEvent(g_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear()
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

    rtvDescriptor = g_deviceResources->GetRenderTargetView();
    dsvDescriptor = g_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, SKY_COLOR, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = g_deviceResources->GetScreenViewport();
    auto scissorRect = g_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

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

    // Setup planets.
    CreatePlanet(SUN_MASS, SUN_DIAMETER, TERRAIN_COLOR, Vector3::Zero, Vector3::Zero, 0);

    const double ratioSizeMass = EARTH_DIAMETER / EARTH_MASS;
    for (int i = 0; i < 1000000; i++)
    {
        const double mass = rand(MOON_MASS / 10, EARTH_MASS * 10);

        Vector3 direction;
        randv(0, 1).Normalize(direction);

        CreatePlanet(
            mass, 
            mass * ratioSizeMass,
            Colors::GhostWhite,
            randv(SUN_DIAMETER, EARTH_SUN_DIST * 2.),
            direction,
            rand(0, EARTH_SUN_VELOCITY * 2.)
        );
    }

    //CreatePlanet(EARTH_MASS, EARTH_DIAMETER, Colors::Red, 
    //    { EARTH_SUN_DIST, 0, 0 },
    //    { 0, 0, 1 }, 
    //    EARTH_SUN_VELOCITY);
    //
    //CreatePlanet(MOON_MASS, MOON_DIAMETER, Colors::Aquamarine,
    //    { EARTH_SUN_DIST - MOON_EARTH_DIST, 0, 0 },
    //    { 0, 0, 1.06 },
    //    EARTH_SUN_VELOCITY + MOON_EARTH_VELOCITY);

    m_planetRenderer = std::make_unique<PlanetRenderer>();

    // Setup grid.
    m_graphic_grid = std::make_unique<Grid>();
    m_graphic_grid->SetColor(GRID_COLOR);

    m_graphic_grid->SetOrigin({ 0, 0, 0 });
    float grid_size = EARTH_SUN_DIST * 2 / S_NORM;
    int grid_divisions = round(grid_size / (SUN_DIAMETER * 2 / S_NORM));
    m_graphic_grid->SetDivisionsAndSize(grid_divisions, grid_size);
}

Planet& Game::CreatePlanet(double mass, double diameter, XMVECTORF32 color, Vector3 position, Vector3 direction, float velocity)
{
    Planet p(mass / M_NORM, diameter / S_NORM, color);
    p.SetPosition(position / S_NORM, direction, (velocity / 3600) / S_NORM);

    g_planets.push_back(p);

    return p;
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

    g_camera->InitProjMatrix(XM_PI / 4.f, backBufferWidth, backBufferHeight, 0.1f, 1000.f);
}

void Game::OnDeviceLost()
{
    m_resourceDescriptors.reset();

    m_graphic_grid.reset();
    g_planets.clear();

    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
