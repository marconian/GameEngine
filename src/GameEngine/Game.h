//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "Globals.h"
#include "StepTimer.h"
#include "Grid.h"
#include "Text.h"
#include "PlanetRenderer.h"
#include "Planet.h"
#include "Camera.h"

using VertexType = DirectX::VertexPositionColor;

// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const;
    Vector3 GetRelativePosition();

private:

    void Update(DX::StepTimer const& timer);
    void Render();
    void RenderMain();
    void RenderInterface();

    void ClearBackBuffers();

    void SetRenderTargetMain();
    void SetRenderTargetMsaa();
    void ResolveRenderTargetMsaa();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void CreateSolarSystem();
    Planet const& CreatePlanet(double mass, double density, double temperature, Vector3 position, Vector3 direction, float velocity);

    bool                                            m_show_grid;
    bool                                            m_changing_planet;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;
    float                                           m_timer_elapsed;
    float                                           m_timer_total;
    float                                           m_elapsed;

    // Input devices.
    std::unique_ptr<DirectX::Keyboard>              m_keyboard;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;
    std::unique_ptr<PlanetRenderer>                 m_planetRenderer;

    DirectX::Keyboard::KeyboardStateTracker         m_keyboardButtons;
    DirectX::Mouse::ButtonStateTracker              m_mouseButtons;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
    std::unique_ptr<Grid>                           m_graphic_grid;
    std::unique_ptr<Text>                           m_if_main;
    std::unique_ptr<Text>                           m_if_composition;


    enum Descriptors
    {
        UIFont,
        CtrlFont,
        Count
    };

    // Other

    DirectX::SimpleMath::Vector3                    m_position;
    float                                           m_pitch;
    float                                           m_yaw;
    float                                           m_zoom;
};
