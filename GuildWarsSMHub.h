//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "ConnectionData.h"
#include "PartyManager.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class GuildWarsSMHub final : public DX::IDeviceNotify
{
public:
    GuildWarsSMHub() noexcept(false);
    ~GuildWarsSMHub() = default;

    GuildWarsSMHub(GuildWarsSMHub&&) = default;
    GuildWarsSMHub& operator=(GuildWarsSMHub&&) = default;

    GuildWarsSMHub(GuildWarsSMHub const&) = delete;
    GuildWarsSMHub& operator=(GuildWarsSMHub const&) = delete;

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
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

    void Terminate();

private:
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer m_timer;

    ConnectionData m_connection_data;
    PartyManager m_party_manager;

    std::thread m_party_manager_thread;
    std::thread m_connection_data_thread;

    std::array<GW_skill, 3432> m_skills;
};
