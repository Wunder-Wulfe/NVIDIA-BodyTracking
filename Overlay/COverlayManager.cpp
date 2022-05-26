#include "pch.h"
#include "COverlayManager.h"

COverlayManager::COverlayManager(CServerDriver *drv) : driver(drv)
{
    m_renderTexture = new sf::RenderTexture();
    if (!m_renderTexture->create(1024U, 512U)) throw std::runtime_error("Unable to create render target for GUI overlay");

    sf::Window l_dummyWindow; // imgui-sfml checks only window focus
    ImGui::SFML::Init(l_dummyWindow, *m_renderTexture, false);
}

COverlayManager::~COverlayManager() {
    Cleanup();
}

void COverlayManager::Init()
{
    vr::EVRInitError l_initError;
    m_vrSystem = vr::VR_Init(&l_initError, vr::VRApplication_Overlay);
    if (l_initError != vr::VRInitError_None)
    {
        std::string l_errorString("Unable to intialize OpenVR: ");
        l_errorString.append(vr::VR_GetVRInitErrorAsEnglishDescription(l_initError));
        throw std::runtime_error(l_errorString);
    }

    vr::VROverlay()->CreateDashboardOverlay("nvidiaBodyTracking.dashboard", "NVIDIA Body Tracking Overlay", &m_dashboardOverlay, &m_dashboardOverlayThumbnail);
    if ((m_dashboardOverlay != vr::k_ulOverlayHandleInvalid) && (m_dashboardOverlayThumbnail != vr::k_ulOverlayHandleInvalid))
    {
        std::string l_fullPath(MAX_PATH, '\0');
        l_fullPath.resize(GetCurrentDirectoryA(MAX_PATH, &l_fullPath[0U]));
        l_fullPath.append("\\..\\..\\resources\\icons\\overlay\\thumb.png");
        vr::VROverlay()->SetOverlayFromFile(m_dashboardOverlayThumbnail, l_fullPath.c_str());

        vr::HmdVector2_t l_mouseScale{ { 1024.f, 512.f } };
        vr::VROverlay()->SetOverlayInputMethod(m_dashboardOverlay, vr::VROverlayInputMethod_Mouse);
        vr::VROverlay()->SetOverlayMouseScale(m_dashboardOverlay, &l_mouseScale);
        vr::VROverlay()->SetOverlayWidthInMeters(m_dashboardOverlay, 3.f);

        m_dashboardTexture.eColorSpace = vr::ColorSpace_Gamma;
        m_dashboardTexture.eType = vr::TextureType_OpenGL;
        m_dashboardTexture.handle = nullptr;
    }

    m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
    m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
    m_event = { 0 };

    m_active = true;
}

void COverlayManager::Cleanup()
{

}

void COverlayManager::Pulse()
{
    while (vr::VROverlay()->PollNextOverlayEvent(m_dashboardOverlay, &m_event, sizeof(vr::VREvent_t)))
    {
        switch (m_event.eventType)
        {
        case vr::VREvent_MouseMove:
        {
            sf::Event l_event;
            l_event.type = sf::Event::EventType::MouseMoved;
            l_event.mouseMove.x = m_event.data.mouse.x;
            l_event.mouseMove.y = m_event.data.mouse.y;
            ImGui::SFML::ProcessEvent(l_event);
        } break;
        case vr::VREvent_MouseButtonDown: case vr::VREvent_MouseButtonUp:
        {
            sf::Event l_event;
            l_event.type = (m_event.eventType == vr::VREvent_MouseButtonDown ? sf::Event::EventType::MouseButtonPressed : sf::Event::EventType::MouseButtonReleased);
            l_event.mouseButton.button = m_event.data.mouse.button == vr::VRMouseButton_Left ? sf::Mouse::Button::Left : sf::Mouse::Button::Right;
            l_event.mouseButton.x = m_event.data.mouse.x;
            l_event.mouseButton.y = m_event.data.mouse.y;
            ImGui::SFML::ProcessEvent(l_event);
        } break;
        }
    }

    if (IsOverlayVisible()) vr::VROverlay()->SetOverlayTexture(m_dashboardOverlay, &m_dashboardTexture);
}

bool COverlayManager::IsOverlayVisible() const
{
    bool l_result = false;
    if (m_vrSystem)
    {
        if (m_dashboardOverlay != vr::k_ulOverlayHandleInvalid) l_result = vr::VROverlay()->IsOverlayVisible(m_dashboardOverlay);
    }
    return l_result;
}

void COverlayManager::SetOverlayTexture(unsigned int p_name)
{
    if (m_dashboardOverlay != vr::k_ulOverlayHandleInvalid) m_dashboardTexture.handle = reinterpret_cast<void *>(static_cast<uintptr_t>(p_name));;
}