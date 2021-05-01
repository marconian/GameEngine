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
	void OnDeviceLost() override;
	void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowMoved();
	void OnWindowSizeChanged(int width, int height);

	// Properties
	static void GetDefaultSize(int& width, int& height);
	DirectX::SimpleMath::Vector3 GetRelativePosition() const;

private:

	void Update(DX::StepTimer const& timer);
	void Render();
	void RenderMain() const;
	void RenderInterface() const;

	void ClearBackBuffers() const;

	void SetRenderTargetMain() const;
	void SetRenderTargetMsaa() const;
	void ResolveRenderTargetMsaa() const;

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources() const;

	void CreateSolarSystem() const;
	static Planet const& CreatePlanet(double mass, double density, double temperature,
	                                  DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector3 direction,
	                                  float velocity);

	bool m_show_grid = false;
	bool m_changing_planet = false;

	// Rendering loop timer.
	DX::StepTimer m_timer;
	float m_timer_elapsed = 0;
	float m_timer_total = 0;
	float m_elapsed = 0;

	// Input devices.
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<PlanetRenderer> m_planetRenderer;

	DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;
	DirectX::Mouse::ButtonStateTracker m_mouseButtons;

	// DirectXTK objects.
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
	std::unique_ptr<Grid> m_graphic_grid;
	std::unique_ptr<Text> m_if_main;
	std::unique_ptr<Text> m_if_composition;


	enum Descriptors
	{
		UIFont,
		CtrlFont,
		Count
	};

	// Other

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
	float m_pitch = 60;
	float m_yaw = 0;
	float m_zoom = DEFAULT_ZOOM;
};
