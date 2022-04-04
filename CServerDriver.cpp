#include "pch.h"

#include "CServerDriver.h"
#include "CDriverSettings.h"
#include "CNvSDKInterface.h"
#include "CVirtualBodyTracker.h"
#include "CVirtualBaseStation.h"
#include "CCameraDriver.h"
#include "CCommon.h"

#define ptrsafe(ptr) if((ptr) == nullptr) return
#define ptrsaferet(ptr, ret) if((ptr) == nullptr) return (ret)

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
    m_frame = 0u;
    m_scaleFactor = glm::vec3(1.f, 1.f, 1.f);
    m_activations = BINDING::NONE;
    m_camBryan = glm::vec3(.0f);
    m_camThread = nullptr;
    mirrored = false;
}

CServerDriver::~CServerDriver()
{
    Cleanup();
}


void CServerDriver::MapBinding(const BINDING &binding, const int &key)
{ 
    m_bindings[binding] = key; 
}
bool CServerDriver::BindingActive(const BINDING &bind) const
{
    return FLAG_ACTIVE(m_activations, bind); 
}
bool CServerDriver::BindingPressed(const BINDING &bind) const
{
    auto it = m_bindings.find(bind);
    if (it == m_bindings.end())
        return false;
    else
        return GetKeyDown(it->second) && !BindingActive(bind);
}
void CServerDriver::UpdateBindings()
{
    m_activations = BINDING::NONE;
    for (const auto &item : m_bindings)
    {
        if (GetKeyDown(item.second))
            m_activations = FLAG_OR(m_activations, item.first);
    }
}

bool CServerDriver::TrackerUpdate(CVirtualBodyTracker &tracker, const CNvSDKInterface &inter, const Proportions &props)
{
    bool confidencePassed = false;
    
    switch (tracker.role)
    {
    case TRACKER_ROLE::HIPS:
        confidencePassed = inter.GetConfidenceAcceptable(BODY_JOINT::PELVIS);
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

    //vr_log("Tracker %s confidence check?", TrackerRoleName[(int)tracker.role]);

    if (!confidencePassed)
        return false;

    //vr_log("Tracker %s passed confidence check", TrackerRoleName[(int)tracker.role]);

    tracker.SetOffsetTransform(inter.GetCameraMatrix());
    tracker.UpdateTransform(inter.GetTransformFromRole(tracker.role));

    //vr_log("Tracker %s updated transform check", TrackerRoleName[(int)tracker.role]);
    //vr_log("transform info: %.3f %.3f %.3f", transform[3][0], transform[3][1], transform[3][2]);

    return true;
}

void CServerDriver::OnImageUpdate(const CCameraDriver &me, cv::Mat image)
{
    CServerDriver *driv = me.driver;
    ptrsafe(driv);
    CNvSDKInterface *track = driv->m_nvInterface;
    ptrsafe(track);
    
    if (track->trackingActive && track->ready)
    {
        //vr_log("Updating the image from the camera (frame %d)\n", driv->m_frame);
        track->UpdateImageFromCam(me.GetImage());
        //vr_log("Computing NVIDIA data (frame %d)\n", driv->m_frame);
        track->RunFrame();
        for (auto tracker : driv->m_trackers)
        {
            //vr_log("Tracker %s is being updated", TrackerRoleName[(int)tracker->role]);
            tracker->SetStandby(!TrackerUpdate(*tracker, *track, *driv->m_proportions));
            //vr_log("CONNECTED? %s", tracker->IsConnected() ? "TRUE" : "FALSE");
        }
    }
    else
    {
        vr_log("Trackers are not ready to be connected (frame %d)\n", driv->m_frame);
        for (auto tracker : driv->m_trackers)
        {
            tracker->SetStandby(true);
        }
    }
}

void CServerDriver::OnCameraUpdate(const CCameraDriver &me, int index)
{
    ptrsafe(me.driver);
    ptrsafe(me.driver->m_nvInterface);

    me.driver->m_nvInterface->SetFPS(me.GetFps());
    vr_log("Attempting to load the image from the camera onto GPU memory\n");
    me.driver->m_nvInterface->LoadImageFromCam(me.m_currentCamera);
    vr_log("Successful in loading image to GPU memory\n");
}

inline const float lua_fmod(const float &a, const float &b)
{
    return a - floor(a / b) * b;
}

template<class T>
void CServerDriver::DoRotateCam(T &axis, const float &amount)
{
    axis = lua_fmod(axis + amount + 180.f, 360.f) - 180.f;
    m_nvInterface->SetCameraRotation(DoEulerYXZ(glm::radians(m_camBryan)));
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    vr_log("Loading settings.ini config file...\n");
    m_driverSettings = new CDriverSettings();
    m_driverSettings->LoadConfig();
    vr_log("settings.ini config file loaded successfully\n");

    m_trackingMode = m_driverSettings->GetConfigTrackingMode(SECTION_TRACK_MODE);
    m_interpolation = m_driverSettings->GetConfigInterpolationMode(SECTION_TRACKSET, KEY_INTERP);
    vr_log("Interpolation mode: %s", InterpModeName[(int)m_interpolation]);
    m_scaleFactor = m_driverSettings->GetConfigVector(SECTION_TRACK_SCALE, glm::vec3(1.f, 1.f, 1.f));
    m_proportions = new Proportions(m_driverSettings->GetConfigProportions(SECTION_TRACKSET));
    mirrored = m_driverSettings->GetConfigBoolean(SECTION_CAMSET, KEY_CAM_MIRROR, false);

    vr_log("Body proportions:");

    vr_log("\tHip offset: %.2f", m_proportions->hipOffset);
    vr_log("\tChest offset: %.2f", m_proportions->chestOffset);
    vr_log("\tElbow offset: %.2f", m_proportions->elbowOffset);
    vr_log("\tKnee offset: %.2f", m_proportions->kneeOffset);
    vr_log("\tFoot offset: %.2f", m_proportions->footOffset);

    vr_log("Loading NVIDIA AR SDK modules...\n");
    try
    {
        m_nvInterface = new CNvSDKInterface();
        m_nvInterface->driver = this;
        m_nvInterface->batchSize = 1;
        m_nvInterface->realBatches = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, 1);
        m_nvInterface->focalLength = m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_FOCAL, 800.0f);
        m_nvInterface->stabilization = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_STABLE, true);
        m_nvInterface->useCudaGraph = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, true);
        m_nvInterface->nvARMode = m_driverSettings->GetConfigInteger(SECTION_SDKSET, KEY_NVAR, 1);
        m_nvInterface->confidenceRequirement = m_driverSettings->GetConfigFloat(SECTION_SDKSET, KEY_CONF, 0.0);
        m_nvInterface->trackingActive = m_driverSettings->GetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, true);
        m_camBryan = m_driverSettings->GetConfigVector(SECTION_ROT);
        m_nvInterface->SetCamera(
            m_driverSettings->GetConfigVector(SECTION_POS), 
            DoEulerYXZ(
                glm::radians(m_camBryan)
            )
        );
        m_nvInterface->Initialize();
        m_nvInterface->KeyInfoUpdated(true);
    }
    catch(std::exception e)
    {
        vr_log("Unable to use NVIDIA AR SDK: %s\n", e.what());
        return vr::EVRInitError::VRInitError_Driver_NotLoaded;
    }
    vr_log("NVIDIA AR SDK modules loaded successfully\n");

    vr_log("Loading OpenCV modules...\n");
    try
    {
        m_cameraDriver = new CCameraDriver(this, m_driverSettings->GetConfigFloat(SECTION_CAMSET, KEY_RES_SCALE, 1.f));
        m_cameraDriver->show = m_driverSettings->GetConfigBoolean(SECTION_CAMSET, KEY_CAM_VIS, true);
        m_cameraDriver->m_cameraIndex = m_driverSettings->GetConfigInteger(SECTION_CAMSET, KEY_CAM_INDEX, 0);
        m_cameraDriver->LoadCameras();
        vr_log("\tBinding events");
        m_cameraDriver->imageChanged += CFunctionFactory(OnImageUpdate, void, const CCameraDriver&, cv::Mat);
        m_cameraDriver->cameraChanged += CFunctionFactory(OnCameraUpdate, void, const CCameraDriver &, int);
        vr_log("\tLaunching camera thread");
        m_camThread = new std::thread(&CCameraDriver::RunAsync, m_cameraDriver);
        vr_log("\tCamera thread launched asynchronously");
    }
    catch(std::exception e)
    {
        vr_log("Unable to use OpenCV: %s\n", e.what());
        return vr::EVRInitError::VRInitError_Driver_NotLoaded;
    }
    vr_log("OpenCV modules loaded successfully\n");

    m_station = new CVirtualBaseStation(this);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_station->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference, m_station);

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

    vr_log("Trackers initialized");

    vr_log("Binding inputs...");


    MapBinding(BINDING::SHIFT, VK_SHIFT);

    vr_log("\tWASD: Move the base station");
    MapBinding(BINDING::MOVE_FORWARD, 'W');
    MapBinding(BINDING::MOVE_LEFT, 'A');
    MapBinding(BINDING::MOVE_BACKWARD, 'S');
    MapBinding(BINDING::MOVE_RIGHT, 'D');

    vr_log("\tE/Q: Raise/lower the base station");
    MapBinding(BINDING::MOVE_UP, 'E');
    MapBinding(BINDING::MOVE_DOWN, 'Q');

 
    vr_log("\tUP/DOWN: Rotate the base station up/down");
    MapBinding(BINDING::PITCH_UP, VK_UP);
    MapBinding(BINDING::PITCH_DOWN, VK_DOWN);

    vr_log("\tLEFT/RIGHT: Rotate the base station left/right");
    MapBinding(BINDING::YAW_LEFT, VK_LEFT);
    MapBinding(BINDING::YAW_RIGHT, VK_RIGHT);

    vr_log("\t,/.: Tilt the base station left/right");
    MapBinding(BINDING::ROLL_LEFT, VK_OEM_COMMA);
    MapBinding(BINDING::ROLL_RIGHT, VK_OEM_PERIOD);

    vr_log("\tT/R: Scale the X axis up/down");
    MapBinding(BINDING::SCALE_X_UP, 'T');
    MapBinding(BINDING::SCALE_X_DOWN, 'R');

    vr_log("\tU/Y: Scale the Y axis up/down");
    MapBinding(BINDING::SCALE_Y_UP, 'U');
    MapBinding(BINDING::SCALE_Y_DOWN, 'Y');

    vr_log("\tO/I: Scale the Z axis up/down");
    MapBinding(BINDING::SCALE_Z_UP, 'O');
    MapBinding(BINDING::SCALE_Z_DOWN, 'I');

    vr_log("\tG/F: Switch to the next/previous camera");
    MapBinding(BINDING::NEXT_CAMERA, 'G');
    MapBinding(BINDING::PREVIOUS_CAMERA, 'F');

    vr_log("\tWASD + SHIFT: Move the HMD offset");

    vr_log("\tZ: Toggle the automatic HMD alignment");
    MapBinding(BINDING::TOGGLE_ALIGN, 'Z');

    vr_log("\tX: Toggle wheter or not to mirror the camera image");
    MapBinding(BINDING::TOGGLE_MIRROR, 'X');

    vr_log("\tCtrl + S: Attempt to save the configuration file");
    MapBinding(BINDING::SAVE_KEY, 'S');
    MapBinding(BINDING::CTRL, VK_CONTROL);

    vr_log("All inputs bound successfully");

    return vr::VRInitError_None;
}

bool CServerDriver::TrySaveConfig() const
{
    int i;
    for (i = 0; i < 5; i++)
    {
        vr_log("Attempting to save config (#%d)...", i);
        m_driverSettings->UpdateConfig(this);
        m_driverSettings->UpdateConfig(this);
        if (m_driverSettings->SaveConfig()) break;
    }
    if (i < 5)
    {
        vr_log("Configuration saved successfully");
        return true;
    }
    else
    {
        vr_log("Configuration failed to save!");
        return false;
    }
}

void CServerDriver::Cleanup()
{
    vr_log("Initiating full device cleanup...\n");

    TrySaveConfig();

    m_trackers.clear();
    
    delptr(m_driverSettings);

    delptr(m_station);

    m_cameraDriver->m_working = false;

    m_cameraDriver->Cleanup();

    delptr(m_nvInterface);
    delptr(m_proportions);

    m_cameraDriver->m_working = false;

    m_camThread->join();

    delptr(m_cameraDriver);
    delptr(m_camThread);

    vr_log("Full device cleanup was successful\n");

    vr::CleanupDriverContext();
}

void CServerDriver::ProcessEvent(const vr::VREvent_t &evnt)
{

}

void CServerDriver::Deactivate()
{
    Cleanup();
}

void CServerDriver::RunFrame()
{
    static bool was_ready = false;
    static double last_clock = systime();
    double cur_clock = systime();
    double clock_diff = cur_clock - last_clock;
    static float move_speed = .25f, rotate_speed = 45.f, scale_speed = .125f;
    float move_amnt, rotate_amnt, scale_amnt;
    static bool camup = false, camdn = false;
    static bool first_time = true;
    vr::VREvent_t ev;

    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, m_hmd_controller_pose, 3);

    m_frame++;

    ptrsafe(m_nvInterface);
    ptrsafe(m_cameraDriver);
    ptrsafe(m_station);

    if (first_time)
    {
        vr_log("HMD Alignment %s", m_nvInterface->m_alignHMD ? "enabled" : "disabled");
        vr_log("Camera %s mirrored", mirrored ? "is" : "is not");
    }

    ptrsafe(m_camThread);

    LoadRefreshRate();
    m_refreshRateCache = (float)(1./clock_diff);
    last_clock = cur_clock;
    LoadFPS();

    move_amnt = (float)(move_speed * clock_diff);
    rotate_amnt = (float)(rotate_speed * clock_diff);
    scale_amnt = (float)(scale_speed * clock_diff);

    while (vr::VRServerDriverHost()->PollNextEvent(&ev, sizeof(vr::VREvent_t)))
       ProcessEvent(ev);

    if (BindingActive(BINDING::SHIFT))
    {
        if (BindingActive(BINDING::MOVE_FORWARD))
            m_nvInterface->m_offset.z += move_amnt;
        if (BindingActive(BINDING::MOVE_BACKWARD))
            m_nvInterface->m_offset.z -= move_amnt;
        if (BindingActive(BINDING::MOVE_LEFT))
            m_nvInterface->m_offset.x += move_amnt;
        if (BindingActive(BINDING::MOVE_RIGHT))
            m_nvInterface->m_offset.x -= move_amnt;

        if (BindingActive(BINDING::MOVE_UP))
            m_nvInterface->m_offset.y += move_amnt;
        if (BindingActive(BINDING::MOVE_DOWN))
            m_nvInterface->m_offset.y -= move_amnt;
    }
    else
    {
        if (BindingActive(BINDING::MOVE_FORWARD))
            m_nvInterface->TranslateCamera(glm::vec3(0.f, 0.f, move_amnt));
        if (BindingActive(BINDING::MOVE_BACKWARD))
            m_nvInterface->TranslateCamera(glm::vec3(0.f, 0.f, -move_amnt));
        if (BindingActive(BINDING::MOVE_LEFT))
            m_nvInterface->TranslateCamera(glm::vec3(move_amnt, 0.f, 0.f));
        if (BindingActive(BINDING::MOVE_RIGHT))
            m_nvInterface->TranslateCamera(glm::vec3(-move_amnt, 0.f, 0.f));

        if (BindingActive(BINDING::MOVE_UP))
            m_nvInterface->TranslateCamera(glm::vec3(0.f, move_amnt, 0.f));
        if (BindingActive(BINDING::MOVE_DOWN))
            m_nvInterface->TranslateCamera(glm::vec3(0, -move_amnt, 0.f));
    }


    if (BindingActive(BINDING::PITCH_UP))
        DoRotateCam(m_camBryan.x, rotate_amnt);
    if (BindingActive(BINDING::PITCH_DOWN))
        DoRotateCam(m_camBryan.x, -rotate_amnt);

    if (BindingActive(BINDING::YAW_LEFT))
        DoRotateCam(m_camBryan.y, -rotate_amnt);
    if (BindingActive(BINDING::YAW_RIGHT))
        DoRotateCam(m_camBryan.y, rotate_amnt);

    if (BindingActive(BINDING::ROLL_LEFT))
        DoRotateCam(m_camBryan.z, rotate_amnt);
    if (BindingActive(BINDING::ROLL_RIGHT))
        DoRotateCam(m_camBryan.z, -rotate_amnt);


    if (BindingActive(BINDING::SCALE_X_UP))
        m_scaleFactor.x += scale_amnt;
    if (BindingActive(BINDING::SCALE_X_DOWN))
        m_scaleFactor.x -= scale_amnt;

    if (BindingActive(BINDING::SCALE_Y_UP))
        m_scaleFactor.y += scale_amnt;
    if (BindingActive(BINDING::SCALE_Y_DOWN))
        m_scaleFactor.y -= scale_amnt;

    if (BindingActive(BINDING::SCALE_Z_UP))
        m_scaleFactor.z += scale_amnt;
    if (BindingActive(BINDING::SCALE_Z_DOWN))
        m_scaleFactor.z -= scale_amnt;

    if (BindingPressed(BINDING::NEXT_CAMERA))
        m_cameraDriver->ChangeCamera(1);
    if (BindingPressed(BINDING::PREVIOUS_CAMERA))
        m_cameraDriver->ChangeCamera(-1);
    if (BindingPressed(BINDING::TOGGLE_ALIGN))
    {
        m_nvInterface->m_alignHMD = !m_nvInterface->m_alignHMD;
        vr_log("HMD Alignment %s", m_nvInterface->m_alignHMD ? "enabled" : "disabled");
    }
    if (BindingPressed(BINDING::TOGGLE_MIRROR))
    {
        mirrored = !mirrored;
        vr_log("Camera %s mirrored", mirrored ? "is" : "is not");
    }

    if (BindingActive(BINDING::CTRL))
    {
        if (BindingPressed(BINDING::SAVE_KEY))
            TrySaveConfig();
    }

    UpdateBindings();

    m_nvInterface->m_axisScale.x = 0.001f * m_scaleFactor.x * (mirrored ? -1.f : 1.f);
    m_nvInterface->m_axisScale.y = -0.001f * m_scaleFactor.y;
    m_nvInterface->m_axisScale.z = -0.001f * m_scaleFactor.z;

    //vr_log("Found FPS and Refresh Rates: %.2f %.2f", GetFPS(), GetRefreshRate());

    //vr_log("Fully rendered the camera");
    if (was_ready != m_nvInterface->ready)
    {
        was_ready = m_nvInterface->ready;
        vr_log("NVIDIA AR SDK is %s\n", was_ready ? "ready and accepting image data" : "currently inactive / disabled");
    }
    if (m_nvInterface->ready)
    {
        for (auto l_tracker : m_trackers)
        {
            l_tracker->RunFrame();
        }
    }
    else if (m_frame % 30u == 0u)
        m_cameraDriver->ChangeCamera(0);

    m_station->SetStandby(!m_nvInterface->trackingActive);
    m_station->SetTransform(m_nvInterface->GetCameraMatrix() * glm::mat4_cast(DoEulerYXZ(0.f, M_PI, 0.f)));
    m_station->RunFrame();

    //LeaveStandby(); 

    first_time = false;
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
    //vr_log("\tTracking for %s %s\n", name, enabled ? "enabled" : "disabled");
    
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
    CVirtualBodyTracker *tracker;
    bool enabled = (m_trackingMode & flag) != TRACKING_FLAG::NONE;
    //vr_log("\tTracking for %s %s\n", name, enabled ? "enabled" : "disabled");
    if(enabled)
    {
        tracker = new CVirtualBodyTracker(m_trackers.size(), role);
        m_trackers.push_back(tracker);
        vr::VRServerDriverHost()->TrackedDeviceAdded(tracker->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, tracker);
        tracker->driver = this;

        tracker = new CVirtualBodyTracker(m_trackers.size(), secondary);
        m_trackers.push_back(tracker);
        vr::VRServerDriverHost()->TrackedDeviceAdded(tracker->GetSerial().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker, tracker);
        tracker->driver = this;
    }
}
