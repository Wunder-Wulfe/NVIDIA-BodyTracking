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


enum class BINDING : uint
{
    NONE = 0b0,

    MOVE_FORWARD = 0b1,
    MOVE_BACKWARD = 0b10,
    MOVE_LEFT = 0b100,
    MOVE_RIGHT = 0b1000,
    MOVE_UP = 0b10000,
    MOVE_DOWN = 0b100000,

    PITCH_UP = 0b1000000,
    PITCH_DOWN = 0b10000000,
    YAW_LEFT = 0b100000000,
    YAW_RIGHT = 0b1000000000,
    ROLL_LEFT = 0b10000000000,
    ROLL_RIGHT = 0b100000000000,

    SCALE_X_UP = 0b1000000000000,
    SCALE_X_DOWN = 0b10000000000000,
    SCALE_Y_UP = 0b100000000000000,
    SCALE_Y_DOWN = 0b1000000000000000,
    SCALE_Z_UP = 0b10000000000000000,
    SCALE_Z_DOWN = 0b100000000000000000,

    NEXT_CAMERA = 0b1000000000000000000,
    PREVIOUS_CAMERA = 0b10000000000000000000,

    TOGGLE_ALIGN = 0b100000000000000000000,
    TOGGLE_MIRROR = 0b1000000000000000000000,
    SAVE_KEY = 0b10000000000000000000000,

    SHIFT = 0b100000000000000000000000,
    CTRL = 0b1000000000000000000000000
};

//  The main class responsible for managing data that is transferred between different classes
class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    static void OnImageUpdate(const CCameraDriver &me, cv::Mat image);
    static void OnCameraUpdate(const CCameraDriver &me, int index);

    static const char *const ms_interfaces[];

    TRACKING_FLAG m_trackingMode;
    bool m_standby;
    std::thread *m_camThread;

    // vr::IServerTrackedDeviceProvider
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;
    void Cleanup() override;
    const char *const *GetInterfaceVersions() override;
    void RunFrame() override;
    bool ShouldBlockStandbyMode() override;
    void EnterStandby() override;
    void LeaveStandby() override;

    CServerDriver(const CServerDriver &that) = delete;
    CServerDriver &operator=(const CServerDriver &that) = delete;

    static bool TrackerUpdate(CVirtualBodyTracker &tracker, const CNvSDKInterface &inter, const Proportions &props);
    void SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role);
    void SetupTracker(const char *name, TRACKING_FLAG flag, TRACKER_ROLE role, TRACKER_ROLE secondary);

    inline void LoadRefreshRate() { m_refreshRateCache = vr::VRSettings()->GetFloat("driver_nvidiaBodyTracking", "displayFrequency"); }

    void ProcessEvent(const vr::VREvent_t &evnt);
    static inline const bool GetKeyDown(const int &key) { return GetAsyncKeyState(key) < 0; }
    void MapBinding(const BINDING &binding, const int &key);
    bool BindingActive(const BINDING &bind) const;
    bool BindingPressed(const BINDING &bind) const;
    void Startup();
    void UpdateBindings();
    BINDING m_activations;
    std::map<BINDING, int> m_bindings;
    
    static inline const glm::quat DoEulerYXZ(const float &x = 0.f, const float &y = 0.f, const float &z = 0.f) {
        return glm::quat(glm::vec3(0.f, y, 0.f)) * glm::quat(glm::vec3(x, 0.f, z));
    }
    static inline const glm::quat DoEulerYXZ(const glm::vec3 &vec) {
        return DoEulerYXZ(vec.x, vec.y, vec.z);
    }

    template<class T>
    void DoRotateCam(T &axis, const float &amount = 0.f);

    bool TrySaveConfig() const;
    
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
    glm::vec3 m_scaleFactor;
    glm::vec3 m_camBryan;
    uint m_frame;
    bool mirrored;
    int camIndex;
    int frameCacheSize;
    bool cacheImmediate;

    vr::TrackedDevicePose_t m_hmd_controller_pose[3]{};

    friend class CDriverSettings;
    friend class CVirtualBodyTracker;
    friend class CNvSDKInterface;
    friend class CCameraDriver;
public:
    void Deactivate();
    inline float GetFPS() const { return m_fpsCache; }
    inline float GetRefreshRate() const { return m_refreshRateCache; }

    CServerDriver();
    ~CServerDriver();
};