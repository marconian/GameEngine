//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Grid.h"
#include "Terrain.h"

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
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>            m_deviceResources;

    // MSAA resources.
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_msaaRenderTarget;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_msaaDepthStencil;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_msaaRTVDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_msaaDSVDescriptorHeap;

    unsigned int                                    m_sampleCount;
    bool                                            m_msaa;
    bool                                            m_show_grid;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

    // Input devices.
    std::unique_ptr<DirectX::Keyboard>              m_keyboard;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;

    DirectX::Keyboard::KeyboardStateTracker         m_keyboardButtons;
    DirectX::Mouse::ButtonStateTracker              m_mouseButtons;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
    std::unique_ptr<Grid>                           m_graphic_grid;
    std::unique_ptr<Terrain>                        m_graphic_terrain;

    std::unique_ptr<DirectX::DescriptorHeap>        m_resourceDescriptors;

    enum Descriptors
    {
        UIFont,
        CtrlFont,
        Count
    };

    // Other
    DirectX::SimpleMath::Matrix                     m_world;
    DirectX::SimpleMath::Matrix                     m_view;
    DirectX::SimpleMath::Matrix                     m_proj;

    DirectX::SimpleMath::Vector3                    m_position;
    float                                           m_pitch;
    float                                           m_yaw;
    float                                           m_zoom;
};
