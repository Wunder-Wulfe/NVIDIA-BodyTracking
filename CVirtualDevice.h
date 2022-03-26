#pragma once

class CServerDriver;

class CVirtualDevice : public vr::ITrackedDeviceServerDriver
{
    vr::DriverPose_t m_pose;
    bool m_connected;
    bool m_forcedConnected;

    CVirtualDevice(const CVirtualDevice &that) = delete;
    CVirtualDevice &operator=(const CVirtualDevice &that) = delete;

    // vr::ITrackedDeviceServerDriver
    vr::EVRInitError Activate(uint32_t unObjectId);
    void Deactivate();
    void EnterStandby();
    void *GetComponent(const char *pchComponentNameAndVersion);
    void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);
    vr::DriverPose_t GetPose();
public:
    CVirtualDevice();
    virtual ~CVirtualDevice();

    const std::string &GetSerial() const;

    bool IsConnected() const;
    void SetConnected(bool p_state);
    void SetForcedConnected(bool p_state);

    void SetInRange(bool p_state);

    void SetPosition(glm::vec3 &pos);
    void SetRotation(glm::quat &quat);

    void SetOffsetPosition(glm::vec3 &pos);
    void SetOffsetRotation(glm::quat &quat);

    bool tracking;

    void RunFrame();
protected:
    vr::PropertyContainerHandle_t m_propertyHandle;
    uint32_t m_trackedDevice;

    std::string m_serial;

    virtual void SetupProperties();
};

