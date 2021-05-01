//
// Game.cpp
//

#include "pch.h"

#include "Constants.h"
#include "Globals.h"
#include "Game.h"

#include <memory>
#include <array>
#include <vector>

extern void ExitGame();

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using Microsoft::WRL::ComPtr;

namespace
{
	//const DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f};
	const XMVECTORF32 GRID_COLOR = Colors::DarkGray;
}

Game::Game() noexcept(false)
{
	g_device_resources->RegisterDeviceNotify(this);
}

Game::~Game()
{
	if (g_device_resources)
		g_device_resources->WaitForGpu();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();

	m_mouse->SetWindow(window);

	g_device_resources->SetWindow(window, width, height);

	g_device_resources->CreateDeviceResources();
	CreateDeviceDependentResources();

	g_device_resources->CreateWindowSizeDependentResources();
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

	m_timer_elapsed = static_cast<float>(timer.GetElapsedSeconds());
	m_timer_total = static_cast<float>(timer.GetTotalSeconds());
	m_elapsed += m_timer_elapsed * g_speed;

	// only update this if not paused
	if (g_speed > 0)
	{
		m_planetRenderer->Update(timer);
	}

	if (m_show_grid)
	{
		m_graphic_grid->SetCellSize(static_cast<float>(g_quadrantSize) * static_cast<float>(S_NORM_INV));
		m_graphic_grid->Update(timer);
	}

	auto mouse = m_mouse->GetState();
	m_mouseButtons.Update(mouse);

	auto kb = m_keyboard->GetState();
	m_keyboardButtons.Update(kb);

	// Draw the scene.
	Planet planet = g_planets[g_current];

	g_current = CleanPlanets();

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
	const bool keyO = m_keyboardButtons.IsKeyPressed(m_keyboard->O);
	const bool key0 = m_keyboardButtons.IsKeyPressed(m_keyboard->D0);
	const bool key1 = m_keyboardButtons.IsKeyPressed(m_keyboard->D1);
	const bool key2 = m_keyboardButtons.IsKeyPressed(m_keyboard->D2);
	const bool key3 = m_keyboardButtons.IsKeyPressed(m_keyboard->D3);
	const bool key4 = m_keyboardButtons.IsKeyPressed(m_keyboard->D4);
	const bool key5 = m_keyboardButtons.IsKeyPressed(m_keyboard->D5);
	const bool key6 = m_keyboardButtons.IsKeyPressed(m_keyboard->D6);

	if (keySpace)
		m_show_grid = !m_show_grid;

	if (keyO)
	{
		g_coreView = !g_coreView;

		UpdateGlobalBuffers();
		m_planetRenderer->Refresh();
	}

	if ((kb.RightAlt || kb.LeftAlt) && keyEnter)
	{
		g_device_resources->ToggleFullScreen();
		g_device_resources->CreateWindowSizeDependentResources();
		CreateWindowSizeDependentResources();
	}

	if (key0) g_speed = 0;
	else if (key1) g_speed = 1;
	else if (key2) g_speed = 10;
	else if (key3) g_speed = 100;
	else if (key4) g_speed = 1000;
	else if (key5) g_speed = 10000;
	else if (key6) g_speed = 100000;
	if (keyPeriod || keyComma)
	{
		float speed = g_speed;
		if (keyPeriod) speed = speed > 0 ? speed * 10.f : 1;
		else if (keyComma) speed > 1 ? speed *= .1f : 0;

		if (speed < 1) speed = 0.;
		else if (speed > 100000) speed = 100000.;

		g_speed = speed;
	}

	if ((keyTab || keyEscape || keyC) && !g_planets.empty())
	{
		if (keyTab)
		{
			const int limit = static_cast<int>(g_planets.size());
			int current = static_cast<int>(g_current);

			if (!kb.LeftShift) current++;
			else current--;

			if (current < 0)
				current = limit + current;
			else if (current >= limit)
				current %= limit;

			g_current = current;
		}
		else if (keyC)
		{
			for (unsigned int i = g_current; i < g_planets.size(); i++)
			{
				const Composition<float>& composition = g_compositions[g_planets[i].id];
				if (composition.Hydrogen < static_cast<float>(EPSILON) && g_planets[i].id != g_planets[g_current].id)
				{
					g_current = i;
					break;
				}
				//if (g_planets[i].collisions > 0 && g_planets[i].id != g_planets[g_current].id)
				//{
				//    g_current = i;
				//    break;
				//}
			}
		}
		else g_current = 0;
	}

	if (g_planets[g_current].id != planet.id)
	{
		m_changing_planet = true;

		planet = g_planets[g_current];

		m_zoom = DEFAULT_ZOOM;
		m_pitch, m_yaw = 0;

		m_planetRenderer->Refresh();
	}

	if (!m_changing_planet) // Block this controls on planet change execution
	{
		if (kb.PageUp || kb.Subtract || kb.OemMinus)
			m_zoom += (m_zoom * 0.1f) * static_cast<float>(MOVEMENT_GAIN);
		if (kb.PageDown || kb.Add || kb.OemPlus)
			m_zoom -= (m_zoom * 0.1f) * static_cast<float>(MOVEMENT_GAIN);

		float move_pitch = 0.f;
		if (kb.Up || kb.W)
			move_pitch = 2.f;
		if (kb.Down || kb.S)
			move_pitch = -2.f;
		move_pitch *= static_cast<float>(ROTATION_GAIN);

		float move_yaw = 0.f;
		if (kb.Left || kb.A)
			move_yaw = 2.f;
		if (kb.Right || kb.D)
			move_yaw = -2.f;
		move_yaw *= static_cast<float>(ROTATION_GAIN);

		if (m_zoom < 0)
			m_zoom = 0;

		m_yaw = fmod(m_yaw + move_yaw, 359.f);
		m_pitch = m_pitch + move_pitch;

		m_pitch = std::max(-90.f, m_pitch);
		m_pitch = std::min(90.f, m_pitch);
	}

	Vector3 moveTo = GetRelativePosition();
	float distance = Vector3::Distance(m_position, moveTo);
	m_position += Vector3::Lerp(Vector3::Zero, moveTo - m_position, cos(distance * m_timer_elapsed));

	if (isnan(m_position.x + m_position.y + m_position.z))
	{
		m_changing_planet = false;
	}

	if (m_changing_planet && static_cast<double>(distance) < planet.GetScreenSize() * 20.)
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

void Game::CreateSolarSystem() const
{
	const UINT noOfPlanets = 400;

	double totalMass = 0;
	std::vector<double> masses;
	for (UINT i = 0; i < noOfPlanets; i++)
	{
		double mass;
		if (rand(0., 1.) < .9)
			mass = rand(MOON_MASS * .001, EARTH_MASS * 10);
		else
			mass = rand(EARTH_MASS * 10, EARTH_MASS * 100.);

		totalMass += mass;
		masses.push_back(mass);
	}

	const double sunMass = SYSTEM_MASS;
	const double sunCoreDensity = 150000;
	const double sunCoreTemperature = 15000000;
	//const double sunVelocity = 150000;
	//Vector3 direction, position;

	Planet const& star = CreatePlanet(sunMass, sunCoreDensity, sunCoreTemperature, Vector3::Zero, Vector3::Zero, 0);
	auto const starRadius = static_cast<double>(star.radius);
	//CreatePlanet(JUPITER_MASS, Vector3::Left * JUPITER_SUN_DIST, Vector3::Forward, JUPITER_SUN_VELOCITY);
	//CreatePlanet(EARTH_MASS, Vector3::Left * EARTH_SUN_DIST, Vector3::Forward, EARTH_SUN_VELOCITY);

	//position = Vector3::Left * SUN_DIAMETER * .8;
	//(-position).Normalize(direction);
	//direction = Vector3(-direction.z, 0, direction.x);
	//sunCoreDensity = sqrt(sqrt(sunMass * .5) / rand(6e5, 7e5));
	//CreatePlanet(sunMass * .5, sunCoreDensity, sunCoreTemperature, position, direction, sunVelocity);
	//
	//position = Vector3::Right * SUN_DIAMETER * .8;
	//(-position).Normalize(direction);
	//direction = Vector3(-direction.z, 0, direction.x);
	//sunCoreDensity = sqrt(sqrt(sunMass * .3) / rand(6e5, 7e5));
	//CreatePlanet(sunMass * .3, sunCoreDensity, sunCoreTemperature, position, direction, sunVelocity);

	const double maxDistance = starRadius * 20.;

	for (UINT i = 0; i < noOfPlanets; i++)
	{
		const double mass = masses[i];
		const double density = sqrt(sqrt(mass) / (mass > 5e25 ? rand(6e5, 7e5) : rand(1e5, 2e5)));
		const double temperature = 1;
		const double velocity = rand(EARTH_SUN_VELOCITY * .01, EARTH_SUN_VELOCITY * 10.);

		double const boundary = starRadius * 1.5;
		Vector3 position;
		bool isOutsideBoundery = false;
		while (!isOutsideBoundery)
		{
			double const rotation = rand(0., 2. * PI);
			double distance = rand(-maxDistance, maxDistance);

			position = XMVector3Transform(randv(-1, 1), XMMatrixRotationAxis(Vector3::UnitY,
			                                                                 static_cast<float>(rotation))) * distance;
			position.y = static_cast<float>(rand(-starRadius, starRadius));

			distance = sqrt(pow(position.x, 2) + pow(position.y, 2) + pow(position.z, 2));
			isOutsideBoundery = distance > boundary;
		}

		Vector3 direction;
		(-position).Normalize(direction);
		direction = Vector3(-direction.z, direction.y, direction.x);
		direction.y = rand(-.05f, .05f);

		CreatePlanet(mass, density, temperature, position, direction, static_cast<float>(velocity));
	}
}

Planet const& Game::CreatePlanet(double mass, double density, double temperature, Vector3 position, Vector3 direction,
                                 float velocity)
{
	Planet& p = g_planets.emplace_back(mass, density, temperature, position * static_cast<float>(S_NORM_INV), direction,
	                                   velocity * static_cast<float>(S_NORM_INV));

	g_compositions[p.id] = {};
	g_compositions[p.id].Randomize(p);

	//if (mass < SUN_MASS / 4.)
	//    g_compositions[p.id].Degenerate(p);

	auto newRadius = p.RadiusByDensity();
	if (newRadius.has_value())
		p.radius = static_cast<float>(newRadius.value());
	else p.radius = 1;

	auto newMass = p.MassByDensity();
	if (newMass.has_value())
		p.mass = static_cast<float>(newMass.value());
	else p.mass = 0;

	std::vector<DepthInfo> profile = g_profiles[p.id];

	const size_t nLayers = profile.size();
	const Composition<double> composition = profile[nLayers - 1].composition;
	p.material.color = composition.GetColor();

	return p;
}

Vector3 Game::GetRelativePosition() const
{
	Planet& planet = g_planets[g_current];

	const float pitch = m_pitch * static_cast<float>(PI_RAD);
	const float yaw = m_yaw * static_cast<float>(PI_RAD);

	const float radius = static_cast<float>(planet.GetScreenSize()) * 2 * m_zoom;
	//Vector3 rotation = Vector3::Zero;// planet.direction;
	//rotation.Normalize();

	const float step_x = cos(yaw) * radius;
	const float step_z = sin(yaw) * radius;
	const float step_y = sin(pitch) * radius;

	const Vector3 current = planet.GetPosition() + Vector3(step_x, step_y, step_z);

	return current;
}

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0) return;

	// Prepare the command list to render a new frame.
	g_device_resources->Prepare(D3D12_RESOURCE_STATE_PRESENT);
	auto* const commandList = g_device_resources->GetCommandList();

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	ClearBackBuffers();
	SetRenderTargetMsaa();
	RenderMain();
	ResolveRenderTargetMsaa();
	SetRenderTargetMain();
	RenderInterface();

	PIXEndEvent(commandList);

	// Show the new frame.
	PIXBeginEvent(g_device_resources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");

	g_device_resources->Present();
	m_graphicsMemory->Commit(g_device_resources->GetCommandQueue());
	g_device_resources->WaitForGpu();

	PIXEndEvent(g_device_resources->GetCommandQueue());
}

void Game::RenderMain() const
{
	auto* const commandList = g_device_resources->GetCommandList();

	if (m_show_grid) m_graphic_grid->Render(commandList);
	m_planetRenderer->Render(commandList);
}

void Game::RenderInterface() const
{
	Planet const planet = g_planets[g_current];
	Composition<float> composition = g_compositions[planet.id];
	composition /= composition.sum();

	const RECT windowSize = g_device_resources->GetOutputSize();
	const auto windowWidth = static_cast<float>(windowSize.right - windowSize.left);
	const auto windowHeight = static_cast<float>(windowSize.bottom - windowSize.top);

	char text[500] = {};

	const double velocity = sqrt(
		pow(static_cast<double>(planet.velocity.x), 2) + pow(static_cast<double>(planet.velocity.y), 2) + pow(
			static_cast<double>(planet.velocity.z), 2)) * S_NORM;
	double distance = sqrt(
		pow(static_cast<double>(planet.position.x), 2) + pow(static_cast<double>(planet.position.y), 2) + pow(
			static_cast<double>(planet.position.z), 2)) * S_NORM;
	distance /= EARTH_SUN_DIST;

	sprintf_s(text,
	          "No. of Planets:  %u\nSpeed:  %u\nTotal Collisions: %u\nCollisions: %u\nRadius: %g km\nMass: %g kg/m3\nVelocity: %g m/s\nDistance: %g AU\nDelta Time: %g\nTotal Time: %g",
	          static_cast<int>(g_planets.size()),
	          static_cast<int>(g_speed),
	          static_cast<int>(g_collisions),
	          static_cast<int>(planet.collisions),
	          static_cast<double>(planet.radius) * .001,
	          static_cast<double>(planet.mass),
	          velocity,
	          distance,
	          static_cast<double>(m_timer_elapsed),
	          static_cast<double>(m_elapsed) * (1 / 86400.)
	);

	m_if_main->Print(text, Vector2(10, 10), Left, Colors::Azure);

	//ELEMENTAL_SYMBOLS
	struct c_info
	{
		std::string name;
		float value = 0.f;
	};

	std::array<c_info, 109> infos{};
	for (int i = 0; i < 109; i++)
	{
		infos[i] = {};
		infos[i].name = ELEMENTAL_SYMBOLS[i];
		infos[i].value = composition.data()[i];
	}

	std::sort(infos.begin(), infos.end(),
	          [](const c_info& a, const c_info& b) { return a.value > b.value; });

	const size_t lines = 20;

	std::string cmps;
	for (size_t i = 0; i < lines; i++)
	{
		char buffer[100] = {};
		sprintf_s(buffer, (infos[i].name + ": %f").c_str(), static_cast<double>(infos[i].value));
		cmps += buffer;
		if (i != lines - 1) cmps += "\n";
	}

	m_if_composition->Print(cmps, Vector2(windowWidth - 10, windowHeight - 10), Right, Colors::Azure);
}

// Helper method to clear the back buffers.
void Game::ClearBackBuffers() const
{
	auto* const commandList = g_device_resources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

	if (MSAA_ENABLED)
	{
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			g_device_resources->GetRenderTargetMsaa(),
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor = g_device_resources->GetRenderTargetViewMsaa();
	const D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor = g_device_resources->GetDepthStencilViewMsaa();

	commandList->ClearRenderTargetView(g_device_resources->GetRenderTargetView(), SKY_COLOR, 0, nullptr);
	commandList->ClearDepthStencilView(g_device_resources->GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,
	                                   nullptr);

	commandList->ClearRenderTargetView(g_device_resources->GetRenderTargetViewMsaa(), SKY_COLOR, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto screenViewport = g_device_resources->GetScreenViewport();
	auto scissorRect = g_device_resources->GetScissorRect();

	// Set the viewport and scissor rect.
	commandList->RSSetViewports(1, &screenViewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	PIXEndEvent(commandList);
}

void Game::SetRenderTargetMain() const
{
	auto* const commandList = g_device_resources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Set Render Target Main");

	auto renderTargetView = g_device_resources->GetRenderTargetView();
	auto depthStencilView = g_device_resources->GetDepthStencilView();

	commandList->OMSetRenderTargets(1,
	                                &renderTargetView, FALSE,
	                                &depthStencilView);

	PIXEndEvent(commandList);
}

void Game::SetRenderTargetMsaa() const
{
	auto* const commandList = g_device_resources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Set Render Target Msaa");

	auto renderTargetView = g_device_resources->GetRenderTargetViewMsaa();
	auto depthStencilView = g_device_resources->GetDepthStencilViewMsaa();

	commandList->OMSetRenderTargets(1,
	                                &renderTargetView, FALSE,
	                                &depthStencilView);

	PIXEndEvent(commandList);
}

void Game::ResolveRenderTargetMsaa() const
{
	auto* const commandList = g_device_resources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Resolve Render Target Msaa");

	auto* backBuffer = g_device_resources->GetRenderTarget();
	auto* backBufferMsaa = g_device_resources->GetRenderTargetMsaa();

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
	auto r = g_device_resources->GetOutputSize();
	g_device_resources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!g_device_resources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();

	// TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height)
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
	ID3D12Device* device = g_device_resources->GetD3DDevice();

	// TODO: Initialize device dependent objects here (independent of window size).
	m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

	CreateGlobalBuffers();
	CreateSolarSystem();

	m_planetRenderer = std::make_unique<PlanetRenderer>();

	// Setup grid.
	m_graphic_grid = std::make_unique<Grid>();
	m_graphic_grid->SetOrigin({0, 0, 0});
	m_graphic_grid->SetSize(static_cast<float>(EARTH_SUN_DIST * 2 * S_NORM_INV));
	m_graphic_grid->SetCellSize(static_cast<float>(g_quadrantSize * S_NORM_INV));
	m_graphic_grid->SetColor(GRID_COLOR);

	m_if_main = std::make_unique<Text>();
	m_if_main->CreateDeviceDependentResources();

	m_if_composition = std::make_unique<Text>();
	m_if_composition->CreateDeviceDependentResources();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources() const
{
	const auto outputSize = g_device_resources->GetOutputSize();
	auto backBufferWidth = std::max<int>(static_cast<int>(outputSize.right - outputSize.left), 1u);
	auto backBufferHeight = std::max<int>(static_cast<int>(outputSize.bottom - outputSize.top), 1u);

	if (backBufferWidth == 1 || backBufferHeight == 1)
	{
		GetDefaultSize(backBufferWidth, backBufferHeight);
	}

	const auto width = static_cast<float>(backBufferWidth);
	const auto height = static_cast<float>(backBufferHeight);

	m_if_main->SetViewport(width, height);
	m_if_composition->SetViewport(width, height);

	g_camera->InitProjMatrix(XM_PI / 4.f, width, height, 0.01f, 10000.f);
}

void Game::OnDeviceLost()
{
	m_graphic_grid.reset();
	m_if_main.reset();
	m_if_composition.reset();
	g_planets.clear();

	m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
