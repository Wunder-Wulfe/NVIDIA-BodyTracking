#include "pch.h"
#include "CVirtualDevice.h"
#include "CServerDriver.h"

CVirtualDevice::CVirtualDevice()
{
    m_connected = false;
    m_forcedConnected = true;
    tracking = false;

    m_pose = { 0 };
    m_pose.poseTimeOffset = -0.011;
    m_pose.qWorldFromDriverRotation.w = 1.0;
    m_pose.qWorldFromDriverRotation.x = .0;
    m_pose.qWorldFromDriverRotation.y = .0;
    m_pose.qWorldFromDriverRotation.z = .0;
    m_pose.qDriverFromHeadRotation.w = 1.0;
    m_pose.qDriverFromHeadRotation.x = .0;
    m_pose.qDriverFromHeadRotation.y = .0;
    m_pose.qDriverFromHeadRotation.z = .0;
    m_pose.vecDriverFromHeadTranslation[0U] = .0;
    m_pose.vecDriverFromHeadTranslation[1U] = .0;
    m_pose.vecDriverFromHeadTranslation[2U] = .0;
    m_pose.poseIsValid = false;
    m_pose.willDriftInYaw = false;
    m_pose.shouldApplyHeadModel = false;
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.deviceIsConnected = false;

    m_propertyHandle = vr::k_ulInvalidPropertyContainer;
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

CVirtualDevice::~CVirtualDevice()
{
}

// vr::ITrackedDeviceServerDriver
vr::EVRInitError CVirtualDevice::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_error = vr::VRInitError_Driver_Failed;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyHandle = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_trackedDevice);

        SetupProperties();

        m_connected = true;
        m_pose.deviceIsConnected = (m_connected && m_forcedConnected);
        m_pose.poseIsValid = true;
        m_pose.result = vr::TrackingResult_Running_OK;

        l_error = vr::VRInitError_None;
    }

    return l_error;
}

void CVirtualDevice::Deactivate()
{
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void CVirtualDevice::EnterStandby()
{
}

void *CVirtualDevice::GetComponent(const char *pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = dynamic_cast<vr::ITrackedDeviceServerDriver *>(this);
    return l_result;
}

void CVirtualDevice::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t CVirtualDevice::GetPose()
{
    return m_pose;
}

// CEmulatedDevice
const std::string &CVirtualDevice::GetSerial() const
{
    return m_serial;
}

bool CVirtualDevice::IsConnected() const
{
    return m_connected;
}

void CVirtualDevice::SetConnected(bool p_state)
{
    m_connected = p_state;
    m_pose.deviceIsConnected = (m_connected && m_forcedConnected);
}

void CVirtualDevice::SetForcedConnected(bool p_state)
{
    m_forcedConnected = p_state;
    m_pose.deviceIsConnected = (m_connected && m_forcedConnected);
}

void CVirtualDevice::SetPosition(glm::vec3 &pos)
{
    m_pose.vecPosition[0U] = pos.x;
    m_pose.vecPosition[1U] = pos.y;
    m_pose.vecPosition[2U] = pos.z;
}

void CVirtualDevice::SetRotation(glm::quat &quat)
{
    m_pose.qRotation.x = quat.x;
    m_pose.qRotation.y = quat.y;
    m_pose.qRotation.z = quat.z;
    m_pose.qRotation.w = quat.w;
}

void CVirtualDevice::SetOffsetPosition(glm::vec3 &pos)
{
    m_pose.vecWorldFromDriverTranslation[0U] = pos.x;
    m_pose.vecWorldFromDriverTranslation[1U] = pos.y;
    m_pose.vecWorldFromDriverTranslation[2U] = pos.z;
}

void CVirtualDevice::SetOffsetRotation(glm::quat &quat)
{
    m_pose.qWorldFromDriverRotation.x = quat.x;
    m_pose.qWorldFromDriverRotation.y = quat.y;
    m_pose.qWorldFromDriverRotation.z = quat.z;
    m_pose.qWorldFromDriverRotation.w = quat.w;
}

void CVirtualDevice::SetupProperties()
{
}

void CVirtualDevice::RunFrame()
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}
