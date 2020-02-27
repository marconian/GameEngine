//
// Game.cpp
//

#include "pch.h"
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
    const DXGI_FORMAT c_backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    const DXGI_FORMAT c_depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    unsigned int c_targetSampleCount = 4;

    const XMVECTORF32 START_POSITION = { 0.f, 0.f, 0.f, 0.f };
    const float ROTATION_GAIN = 0.004f;
    const float MOVEMENT_GAIN = 0.1f;

    //const DirectX::XMVECTORF32 SKY_COLOR = { 0.90196f, 0.98824f, 0.85098f, 1.0f};
    const DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f};
    const DirectX::XMVECTORF32 GRID_COLOR = Colors::DarkGray;
    const DirectX::XMVECTORF32 TERRAIN_COLOR = Colors::SaddleBrown;
}

Game::Game() noexcept(false) :
    m_sampleCount(4),
    m_msaa(true),
    m_pitch(0),
    m_yaw(0)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(
        c_backBufferFormat,
        c_depthBufferFormat, /* If we were only doing MSAA rendering, we could skip the non-MSAA depth/stencil buffer with DXGI_FORMAT_UNKNOWN */
        2, 
        4, 
        D3D_FEATURE_LEVEL_11_0, 
        true,
        SKY_COLOR
    );
    m_deviceResources->RegisterDeviceNotify(this);

    m_position = START_POSITION.v;
    m_zoom = 10.f;
    m_show_grid = true;
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();

    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    
    /*m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);*/
    
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

    //auto mouse = m_mouse->GetState();
    //m_mouseButtons.Update(mouse);

    //if (mouse.positionMode == Mouse::MODE_RELATIVE)
    //{
    //    Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f)
    //        * ROTATION_GAIN;

    //    m_pitch -= delta.y;
    //    m_yaw -= delta.x;

    //    // limit pitch to straight up or straight down
    //    // with a little fudge-factor to avoid gimbal lock
    //    float limit = XM_PI / 2.0f - 0.01f;
    //    m_pitch = std::max(-limit, m_pitch);
    //    m_pitch = std::min(+limit, m_pitch);

    //    // keep longitude in sane range by wrapping
    //    if (m_yaw > XM_PI)
    //        m_yaw -= XM_PI * 2.0f;
    //    else if (m_yaw < -XM_PI)
    //        m_yaw += XM_PI * 2.0f;
    //}

    //if (mouse.leftButton)
    //    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    //else
    //    m_mouse->SetMode(Mouse::MODE_ABSOLUTE);

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    //if (kb.Escape)
    //    ExitGame();

    if (m_keyboardButtons.IsKeyPressed(m_keyboard->Home))
    {
        m_position = START_POSITION.v;
        m_pitch = m_yaw = 0.f;
    }
    if (m_keyboardButtons.IsKeyPressed(m_keyboard->Space))
        m_show_grid = !m_show_grid;

    if (kb.PageUp || kb.Subtract || kb.OemMinus)
        m_zoom += 1.f * MOVEMENT_GAIN;
    if (kb.PageDown || kb.Add || kb.OemPlus)
        m_zoom -= 1.f * MOVEMENT_GAIN;

    float move_pitch = 1.f;
    if (kb.Left || kb.A)
        move_pitch = 2.f;
    if (kb.Right || kb.D)
        move_pitch = -2.f;

    if (m_zoom < .1f)
        m_zoom = .1f;

    m_pitch = fmod(m_pitch + move_pitch * MOVEMENT_GAIN, 360);

    float r = 1.0f;
    float a = m_pitch * XM_PI / 180.f;

    float x = 0.f + r * cosf(a);
    float z = 0.f + r * sinf(a);
    float y = 0.f;

    m_position = { x, y, z };

    // Draw the scene.
    m_view = Matrix::CreateLookAt(Vector3(m_zoom, m_zoom, m_zoom),
        Vector3::Zero, Vector3::UnitY);
    m_world = XMMatrixLookAtRH(m_position, Vector3::Zero, Vector3::UnitY);

    if (m_show_grid) m_graphic_grid->Apply(m_proj, m_view, m_world);
    m_graphic_terrain->Apply(m_proj, m_view, m_world);

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
        return;

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare(m_msaa ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    if (m_show_grid) m_graphic_grid->Render(commandList);
    m_graphic_terrain->Render(commandList);

    PIXEndEvent(commandList);

    if (m_msaa)
    {
        // Resolve the MSAA render target.
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Resolve");

        auto backBuffer = m_deviceResources->GetRenderTarget();
        auto backBufferMsaa = m_deviceResources->GetRenderTargetMsaa();

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

        commandList->ResolveSubresource(backBuffer, 0, backBufferMsaa, 0, c_backBufferFormat);

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
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor, dsvDescriptor;
    if (m_msaa)
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_deviceResources->GetRenderTargetMsaa(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->ResourceBarrier(1, &barrier);
    }

    rtvDescriptor = m_deviceResources->GetRenderTargetView();
    dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, SKY_COLOR, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
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
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
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
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    m_graphicsMemory = make_unique<GraphicsMemory>(device);

    // Setup test scene.
    m_graphic_grid = std::make_unique<Grid>(device, c_backBufferFormat, c_depthBufferFormat, m_msaa, m_sampleCount);
    m_graphic_grid->SetOrigin({ 0, 0, 0 });
    m_graphic_grid->SetDivisionsAndSize(1000, 100);
    m_graphic_grid->SetColor(GRID_COLOR);

    // Setup test scene.
    m_graphic_terrain = std::make_unique<Terrain>(device, c_backBufferFormat, c_depthBufferFormat, m_msaa, m_sampleCount);
    m_graphic_terrain->SetOrigin({ 0, 0, 0 });
    m_graphic_terrain->SetDivisionsAndSize(1000, 100);
    m_graphic_terrain->SetColor(TERRAIN_COLOR);

    m_world = Matrix::Identity;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto outputSize = m_deviceResources->GetOutputSize();
    int backBufferWidth = std::max<int>(static_cast<int>(outputSize.right - outputSize.left), 1u);
    int backBufferHeight = std::max<int>(static_cast<int>(outputSize.bottom - outputSize.top), 1u);

    if (backBufferWidth == 1 || backBufferHeight == 1) {
        GetDefaultSize(backBufferWidth, backBufferHeight);
    }

    m_view = Matrix::CreateLookAt(Vector3(Vector3(m_zoom, m_zoom, m_zoom)),
        Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(backBufferWidth) / float(backBufferHeight), 0.1f, 100.f);
}

void Game::OnDeviceLost()
{
    //m_msaaRenderTarget.Reset();
    //m_msaaDepthStencil.Reset();
    //
    //m_msaaRTVDescriptorHeap.Reset();
    //m_msaaDSVDescriptorHeap.Reset();

    m_resourceDescriptors.reset();

    m_graphic_grid.reset();
    m_graphic_terrain.reset();

    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
