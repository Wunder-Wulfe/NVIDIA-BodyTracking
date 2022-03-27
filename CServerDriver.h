#pragma once

class CDriverSettings;
class CNvSDKInterface;
class CVirtualBodyTracker;
class CVirtualBaseStation;
class CCameraDriver;
enum class TRACKING_FLAG;
enum class TRACKER_ROLE;
enum class INTERP_MODE;
struct Proportions;

//  The main class responsible for managing data that is transferred between different classes
class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    static void OnImageUpdate(const CCameraDriver &me, cv::Mat &image);
    static void OnCameraUpdate(const CCameraDriver &me, int index);

    static const char *const ms_interfaces[];

    TRACKING_FLAG m_trackingMode;
    bool m_standby;
    std::thread m_camThread;

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

    static bool TrackerUpdate(CVirtualBodyTracker &tracker, const CNvSDKInterface &inter, const Proportions &props);

    inline void LoadRefreshRate() { m_refreshRateCache = vr::VRSettings()->GetFloat("driver_nvidiaBodyTracking", "displayFrequency"); }
    void LoadFPS();
protected:
    CDriverSettings *m_driverSettings;
    CNvSDKInterface *m_nvInterface;
    std::vector<CVirtualBodyTracker *> m_trackers;
    CVirtualBaseStation *m_station;
    CCameraDriver *m_cameraDriver;
    Proportions *m_proportions;

    INTERP_MODE m_interpolation;

    float m_refreshRateCache;
    float m_fpsCache;
    uint m_frame;

    friend class CDriverSettings;
    friend class CVirtualBodyTracker;
    friend class CNvSDKInterface;
public:
    inline float GetFPS() const { return m_fpsCache; }
    inline float GetRefreshRate() const { return m_refreshRateCache; }

    CServerDriver();
    ~CServerDriver();
};