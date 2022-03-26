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

    void SetPosition(const glm::vec3 &pos);
    void SetRotation(const glm::quat &quat);
    void SetTransform(const glm::vec3 &pos, const glm::quat &quat);
    void SetTransform(const glm::mat4x4 &mat);

    void SetOffsetPosition(const glm::vec3 &pos);
    void SetOffsetRotation(const glm::quat &quat);
    void SetOffsetTransform(const glm::vec3 &pos, const glm::quat &quat);
    void SetOffsetTransform(const glm::mat4x4 &mat);

    void RunFrame();
protected:
    vr::PropertyContainerHandle_t m_propertyHandle;
    uint32_t m_trackedDevice;

    std::string m_serial;

    virtual void SetupProperties();
};

