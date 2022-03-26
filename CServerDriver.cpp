#include "pch.h"

#include "CServerDriver.h"
#include "CDriverSettings.h"
#include "CNvBodyTracker.h"
#include "CVirtualBodyTracker.h"
#include "CVirtualBaseStation.h"
#include "CCameraDriver.h"
#include "CCommon.h"

const char *const CServerDriver::ms_interfaces[]
{
    vr::ITrackedDeviceServerDriver_Version,
    vr::IServerTrackedDeviceProvider_Version,
    nullptr
};

CServerDriver::CServerDriver()
{
    m_driverSettings = nullptr;
    m_bodyTracker = nullptr;
    m_cameraDriver = nullptr;
    m_station = nullptr;
    m_standby = false;
    m_trackingMode = TRACKING_FLAG::NONE;
}

CServerDriver::~CServerDriver()
{
}

void CServerDriver::OnImageUpdate(const CCameraDriver &me, cv::Mat &image)
{
    CServerDriver *driv = me.driver;
    CNvBodyTracker *track = driv->m_bodyTracker;

    if (track->trackingActive)
    {
        for (auto tracker : driv->m_trackers)
        {
            switch (tracker->role)
            {
            case TRACKER_ROLE::HIPS:
                track->GetConfidenceAcceptable(BODY_JOINT::LEFT_HIP);
                break;
            default:
                break;
            }
        }
    }
    else
    {
        for (auto tracker : driv->m_trackers)
        {
            tracker->SetConnected(false);
        }
    }
}

void CServerDriver::OnCameraUpdate(const CCameraDriver &me, int index)
{
    me.driver->m_bodyTracker->SetFPS(me.GetFps());
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    m_driverSettings = new CDriverSettings();
    m_driverSettings->LoadConfig();

    m_trackingMode = m_driverSettings->GetConfigTrackingMode(SECTION_TRACK_MODE);

    try
    {
        m_bodyTracker = new CNvBodyTracker();
        m_bodyTracker->batchSize = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, 1);
        m_bodyTracker->focalLength = m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_FOCAL, 800.0f);
        m_bodyTracker->stabilization = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_STABLE, true);
        m_bodyTracker->useCudaGraph = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, true);
        m_bodyTracker->nvARMode = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_NVAR, 1);
        m_bodyTracker->confidenceRequirement = m_driverSettings->GetConfigFloat(SECTION_SDKSET, KEY_CONF, 0.0);
        m_bodyTracker->trackingActive = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, false);
        m_bodyTracker->SetCamera(m_driverSettings->GetConfigVector(SECTION_POS), m_driverSettings->GetConfigQuaternion(SECTION_ROT));
        m_bodyTracker->KeyInfoUpdated();
    }
    catch(std::exception e)
    {
        vr_log("Unable to use NVIDIA AR SDK: %s\n", e.what());
        return vr::EVRInitError::VRInitError_Driver_NotLoaded;
    }

    try
    {
        m_cameraDriver = new CCameraDriver(this, m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_RES_SCALE, 1.0));
        m_cameraDriver->show = m_driverSettings->GetConfigBoolean(SECTION_CAMSET, KEY_CAM_VIS, true);
        m_cameraDriver->LoadCameras();
        m_cameraDriver->imageChanged += CFunctionFactory(OnImageUpdate, void, const CCameraDriver&, cv::Mat&);
        m_cameraDriver->cameraChanged += CFunctionFactory(OnCameraUpdate, void, const CCameraDriver &, int);
    }
    catch(std::exception e)
    {
        vr_log("Unable to use OpenCV: %s\n", e.what());
        return vr::EVRInitError::VRInitError_Driver_NotLoaded;
    }

    m_station = new CVirtualBaseStation(this);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_station->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference, m_station);

    // Enable body tracking
    m_bodyTracker->Initialize();

    vr_log("Tracking enabled: %s\n", m_bodyTracker->trackingActive ? "true" : "false");
    vr_log("Current tracking modes:\n");

    SetupTracker(KEY_HIP_ON, TRACKING_FLAG::HIP, TRACKER_ROLE::HIPS);
    SetupTracker(KEY_FEET_ON, TRACKING_FLAG::FEET, TRACKER_ROLE::LEFT_FOOT, TRACKER_ROLE::RIGHT_FOOT);
    SetupTracker(KEY_ELBOW_ON, TRACKING_FLAG::ELBOW, TRACKER_ROLE::LEFT_ELBOW, TRACKER_ROLE::RIGHT_ELBOW);
    SetupTracker(KEY_KNEE_ON, TRACKING_FLAG::KNEE, TRACKER_ROLE::LEFT_KNEE, TRACKER_ROLE::RIGHT_KNEE);
    SetupTracker(KEY_CHEST_ON, TRACKING_FLAG::CHEST, TRACKER_ROLE::CHEST);
    SetupTracker(KEY_SHOULDER_ON, TRACKING_FLAG::SHOULDER, TRACKER_ROLE::LEFT_SHOULDER, TRACKER_ROLE::RIGHT_SHOULDER);
    SetupTracker(KEY_TOE_ON, TRACKING_FLAG::TOE, TRACKER_ROLE::LEFT_TOE, TRACKER_ROLE::RIGHT_TOE);
    SetupTracker(KEY_HEAD_ON, TRACKING_FLAG::HEAD, TRACKER_ROLE::HEAD);
    SetupTracker(KEY_HAND_ON, TRACKING_FLAG::HAND, TRACKER_ROLE::LEFT_HAND, TRACKER_ROLE::RIGHT_HAND);

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    for(auto l_tracker : m_trackers)
        delptr(l_tracker);
    m_trackers.clear();

    delptr(m_station);

    delptr(m_cameraDriver);
    delptr(m_bodyTracker);

    m_driverSettings->UpdateConfig(this);
    m_driverSettings->SaveConfig();
    delptr(m_driverSettings);

    vr::CleanupDriverContext();
}

void CServerDriver::RunFrame()
{
    m_bodyTracker->trackingActive = !m_standby;
    m_cameraDriver->RunFrame();
    m_bodyTracker->RunFrame();

    for(auto l_tracker : m_trackers)
    {
        l_tracker->SetConnected(m_bodyTracker->trackingActive);
        l_tracker->SetInRange(m_bodyTracker->GetConfidenceAcceptable());
        l_tracker->RunFrame();
    }

    m_station->SetConnected(m_bodyTracker->trackingActive);
    m_station->RunFrame();
}

void CServerDriver::EnterStandby()
{
    m_standby = true;
}

void CServerDriver::LeaveStandby()
{
    m_standby = false;
}

bool CServerDriver::ShouldBlockStandbyMode()
{
    return false;
}

const char *const *CServerDriver::GetInterfaceVersions()
{
    return ms_interfaces;
}

void CServerDriver::SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role)
{
    bool enabled = (m_trackingMode & flag) != TRACKING_FLAG::NONE;
    vr_log("\tTracking for %s %s\n", name, enabled ? "enabled" : "disabled");
    if(enabled)
    {
        m_trackers.push_back(new CVirtualBodyTracker(m_trackers.size(), role));
        vr::VRServerDriverHost()->TrackedDeviceAdded(m_trackers.back()->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, m_trackers.back());
    }
}

void CServerDriver::SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role, TRACKER_ROLE secondary)
{
    bool enabled = (m_trackingMode & flag) != TRACKING_FLAG::NONE;
    vr_log("\tTracking for %s %s\n", name, enabled ? "enabled" : "disabled");
    if(enabled)
    {
        m_trackers.push_back(new CVirtualBodyTracker(m_trackers.size(), role));
        vr::VRServerDriverHost()->TrackedDeviceAdded(m_trackers.back()->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, m_trackers.back());

        m_trackers.push_back(new CVirtualBodyTracker(m_trackers.size(), secondary));
        vr::VRServerDriverHost()->TrackedDeviceAdded(m_trackers.back()->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, m_trackers.back());
    }
}
