#include "pch.h"

#include "CServerDriver.h"
#include "CDriverSettings.h"
#include "CNvSDKInterface.h"
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

void CServerDriver::LoadFPS()
{ 
    m_fpsCache = m_cameraDriver->GetFps(); 
}

CServerDriver::CServerDriver()
{
    m_driverSettings = nullptr;
    m_nvInterface = nullptr;
    m_cameraDriver = nullptr;
    m_station = nullptr;
    m_standby = false;
    m_trackingMode = TRACKING_FLAG::NONE;
    m_interpolation = INTERP_MODE::NONE;
    m_fpsCache = 30.f;
    m_refreshRateCache = 60.f;
    m_proportions = nullptr;
}

CServerDriver::~CServerDriver()
{
    Cleanup();
}

bool CServerDriver::TrackerUpdate(CVirtualBodyTracker &tracker, const CNvSDKInterface &inter, const Proportions &props)
{
    bool confidencePassed = false;
    switch (tracker.role)
    {
    case TRACKER_ROLE::HIPS:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_HIP, BODY_JOINT::RIGHT_HIP);
        break;
    case TRACKER_ROLE::LEFT_FOOT:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_ANKLE);
        break;
    case TRACKER_ROLE::RIGHT_FOOT:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_ANKLE);
        break;
    case TRACKER_ROLE::LEFT_ELBOW:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_ELBOW);
        break;
    case TRACKER_ROLE::RIGHT_ELBOW:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_ELBOW);
        break;
    case TRACKER_ROLE::LEFT_KNEE:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_KNEE);
        break;
    case TRACKER_ROLE::RIGHT_KNEE:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_KNEE);
        break;
    case TRACKER_ROLE::CHEST:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::TORSO);
        break;
    case TRACKER_ROLE::LEFT_SHOULDER:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_SHOULDER);
        break;
    case TRACKER_ROLE::RIGHT_SHOULDER:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_SHOULDER);
        break;
    case TRACKER_ROLE::LEFT_TOE:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_BIG_TOE, BODY_JOINT::LEFT_SMALL_TOE);
        break;
    case TRACKER_ROLE::RIGHT_TOE:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_BIG_TOE, BODY_JOINT::RIGHT_SMALL_TOE);
        break;
    case TRACKER_ROLE::HEAD:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::NOSE, BODY_JOINT::NECK);
        break;
    case TRACKER_ROLE::LEFT_HAND:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::LEFT_WRIST);
        break;
    case TRACKER_ROLE::RIGHT_HAND:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::RIGHT_WRIST);
        break;
    }

    if (!confidencePassed)
        return false;

    glm::mat4x4 transform;

    switch (tracker.role)
    {
    case TRACKER_ROLE::HIPS:
        transform = inter.CastMatrix(
            glm::mix(
                inter.GetPosition(
                    BODY_JOINT::LEFT_HIP,
                    BODY_JOINT::RIGHT_HIP
                ),
                inter.GetPosition(BODY_JOINT::TORSO),
                props.hipOffset
            ),
            inter.GetRotation(
                BODY_JOINT::LEFT_HIP,
                BODY_JOINT::RIGHT_HIP
            )
        );
        break;
    case TRACKER_ROLE::LEFT_FOOT:
        transform = inter.GetTransform(BODY_JOINT::LEFT_ANKLE, BODY_JOINT::LEFT_HEEL);
        break;
    case TRACKER_ROLE::RIGHT_FOOT:
        transform = inter.GetTransform(BODY_JOINT::RIGHT_ANKLE, BODY_JOINT::RIGHT_HEEL);
        break;
    case TRACKER_ROLE::LEFT_ELBOW:
        transform = inter.GetInterpolatedTransformMulti(
            BODY_JOINT::LEFT_WRIST,
            BODY_JOINT::LEFT_ELBOW,
            BODY_JOINT::LEFT_SHOULDER,
            props.elbowOffset
        );
        break;
    case TRACKER_ROLE::RIGHT_ELBOW:
        transform = inter.GetInterpolatedTransformMulti(
            BODY_JOINT::RIGHT_WRIST,
            BODY_JOINT::RIGHT_ELBOW,
            BODY_JOINT::RIGHT_SHOULDER,
            props.elbowOffset
        );
        break;
    case TRACKER_ROLE::LEFT_KNEE:
        transform = inter.GetInterpolatedTransformMulti(
            BODY_JOINT::LEFT_ANKLE,
            BODY_JOINT::LEFT_KNEE,
            BODY_JOINT::LEFT_HIP,
            props.kneeOffset
        );
        break;
    case TRACKER_ROLE::RIGHT_KNEE:
        transform = inter.GetInterpolatedTransformMulti(
            BODY_JOINT::RIGHT_ANKLE,
            BODY_JOINT::RIGHT_KNEE,
            BODY_JOINT::RIGHT_HIP,
            props.kneeOffset
        );
        break;
    case TRACKER_ROLE::CHEST:
        transform = inter.CastMatrix(
            glm::mix(
                inter.GetPosition(BODY_JOINT::TORSO),
                inter.GetPosition(
                    BODY_JOINT::LEFT_HIP,
                    BODY_JOINT::RIGHT_HIP
                ),
                props.chestOffset
            ),
            inter.GetRotation(BODY_JOINT::TORSO)
        );
        break;
    case TRACKER_ROLE::LEFT_SHOULDER:
        transform = inter.GetTransform(BODY_JOINT::LEFT_SHOULDER);
        break;
    case TRACKER_ROLE::RIGHT_SHOULDER:
        transform = inter.GetTransform(BODY_JOINT::RIGHT_SHOULDER);
        break;
    case TRACKER_ROLE::LEFT_TOE:
        transform = inter.GetAverageTransform(BODY_JOINT::LEFT_BIG_TOE, BODY_JOINT::LEFT_SMALL_TOE);
        break;
    case TRACKER_ROLE::RIGHT_TOE:
        transform = inter.GetAverageTransform(BODY_JOINT::RIGHT_BIG_TOE, BODY_JOINT::RIGHT_SMALL_TOE);
        break;
    case TRACKER_ROLE::HEAD:
        transform = inter.GetTransform(BODY_JOINT::NECK, BODY_JOINT::NOSE);
        break;
    case TRACKER_ROLE::LEFT_HAND:
        transform = inter.GetTransform(BODY_JOINT::LEFT_WRIST);
        break;
    case TRACKER_ROLE::RIGHT_HAND:
        transform = inter.GetTransform(BODY_JOINT::RIGHT_WRIST);
        break;
    }

    tracker.UpdateTransform(transform);
}

void CServerDriver::OnImageUpdate(const CCameraDriver &me, cv::Mat &image)
{
    CServerDriver *driv = me.driver;
    CNvSDKInterface *track = driv->m_nvInterface;

    if (track->trackingActive)
    {
        me.driver->m_nvInterface->UpdateImageFromCam(image);
        me.driver->m_nvInterface->RunFrame();
        for (auto tracker : driv->m_trackers)
        {
            tracker->SetConnected(TrackerUpdate(*tracker, *me.driver->m_nvInterface, *me.driver->m_proportions));
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
    me.driver->m_nvInterface->SetFPS(me.GetFps());
    me.driver->m_nvInterface->LoadImageFromCam(me.m_currentCamera);
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    m_driverSettings = new CDriverSettings();
    m_driverSettings->LoadConfig();

    m_trackingMode = m_driverSettings->GetConfigTrackingMode(SECTION_TRACK_MODE);
    m_interpolation = m_driverSettings->GetConfigInterpolationMode(SECTION_TRACKSET, KEY_INTERP);
    m_proportions = new Proportions(m_driverSettings->GetConfigProportions(SECTION_TRACKSET));

    try
    {
        m_nvInterface = new CNvSDKInterface();
        m_nvInterface->driver = this;
        m_nvInterface->batchSize = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, 1);
        m_nvInterface->focalLength = m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_FOCAL, 800.0f);
        m_nvInterface->stabilization = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_STABLE, true);
        m_nvInterface->useCudaGraph = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, true);
        m_nvInterface->nvARMode = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_NVAR, 1);
        m_nvInterface->confidenceRequirement = m_driverSettings->GetConfigFloat(SECTION_SDKSET, KEY_CONF, 0.0);
        m_nvInterface->trackingActive = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, false);
        m_nvInterface->SetCamera(m_driverSettings->GetConfigVector(SECTION_POS), m_driverSettings->GetConfigQuaternion(SECTION_ROT));
        m_nvInterface->KeyInfoUpdated();
    }
    catch(std::exception e)
    {
        vr_log("Unable to use NVIDIA AR SDK: %s\n", e.what());
        return vr::EVRInitError::VRInitError_Driver_NotLoaded;
    }

    try
    {
        m_cameraDriver = new CCameraDriver(this, m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_RES_SCALE, 1.f));
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
    m_nvInterface->Initialize();

    vr_log("Tracking enabled: %s\n", m_nvInterface->trackingActive ? "true" : "false");
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
    delptr(m_nvInterface);
    delptr(m_proportions);

    m_driverSettings->UpdateConfig(this);
    m_driverSettings->SaveConfig();
    delptr(m_driverSettings);

    vr::CleanupDriverContext();
}

void CServerDriver::RunFrame()
{
    LoadRefreshRate();
    LoadFPS();

    m_nvInterface->trackingActive = !m_standby;
    m_cameraDriver->RunFrame();

    for(auto l_tracker : m_trackers)
    {
        l_tracker->SetConnected(m_nvInterface->trackingActive);
        l_tracker->SetInRange(m_nvInterface->GetConfidenceAcceptable());
        l_tracker->RunFrame();
    }

    m_station->SetConnected(m_nvInterface->trackingActive);
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
    CVirtualBodyTracker *tracker;
    bool enabled = (m_trackingMode & flag) != TRACKING_FLAG::NONE;
    vr_log("\tTracking for %s %s\n", name, enabled ? "enabled" : "disabled");
    
    if(enabled)
    {
        tracker = new CVirtualBodyTracker(m_trackers.size(), role);
        m_trackers.push_back(tracker);
        vr::VRServerDriverHost()->TrackedDeviceAdded(tracker->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, tracker);
        tracker->driver = this;
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
