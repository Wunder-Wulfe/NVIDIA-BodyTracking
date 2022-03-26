#pragma once

class CDriverSettings;
class CNvBodyTracker;
class CVirtualBodyTracker;
class CVirtualBaseStation;
class CCameraDriver;
enum class TRACKING_FLAG;
enum class TRACKER_ROLE;

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    static void OnImageUpdate(const CCameraDriver &me, cv::Mat &image);
    static void OnCameraUpdate(const CCameraDriver &me, int index);

    static const char *const ms_interfaces[];

    TRACKING_FLAG m_trackingMode;
    bool m_standby;

    // vr::IServerTrackedDeviceProvider
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    void Cleanup();
    const char *const *GetInterfaceVersions();
    void RunFrame();
    bool ShouldBlockStandbyMode();
    void EnterStandby();
    void LeaveStandby();

    CServerDriver(const CServerDriver &that) = delete;
    CServerDriver &operator=(const CServerDriver &that) = delete;

    void SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role);
    void SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role, TRACKER_ROLE secondary);
protected:
    CDriverSettings *m_driverSettings;
    CNvBodyTracker *m_bodyTracker;
    std::vector<CVirtualBodyTracker *> m_trackers;
    CVirtualBaseStation *m_station;
    CCameraDriver *m_cameraDriver;
public:
    CServerDriver();
    ~CServerDriver();
};