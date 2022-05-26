#pragma once

class CServerDriver;

class COverlayManager
{
    vr::IVRSystem *m_vrSystem;

    vr::VROverlayHandle_t m_dashboardOverlay;
    vr::VROverlayHandle_t m_dashboardOverlayThumbnail;
    vr::Texture_t m_dashboardTexture;

    vr::TrackedDeviceIndex_t m_kinectDevice;
    vr::TrackedDeviceIndex_t m_leftHand;
    vr::TrackedDeviceIndex_t m_rightHand;

    vr::VREvent_t m_event;

    sf::RenderTexture *m_renderTexture;
    
public:
    CServerDriver *driver;

    bool m_active;

    void Init();
    void Cleanup();
    void Pulse();
    void SetOverlayTexture(unsigned int p_name);
    bool IsOverlayVisible() const;

    COverlayManager(CServerDriver *dr);
    ~COverlayManager();
};

