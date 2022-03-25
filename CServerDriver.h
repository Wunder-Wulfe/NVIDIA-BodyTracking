#pragma once

#include "CBodyTrackDriver.h"

#define BUFFER_SIZE 20

// X Coordinate of a vector/quaternion
#define C_X "X"
// Y Coordinate of a vector/quaternion
#define C_Y "Y"
// Z Coordinate of a vector/quaternion
#define C_Z "Z"
// W Coordinate of a quaternion
#define C_W "W"

// Settings file path
#define C_SETTINGS "./settings.ini"

// Camera position section
#define SECTION_POS "CameraPosition"
// Camera rotation section
#define SECTION_ROT "CameraRotation"

// Camera settings section
#define SECTION_CAMSET "CameraSettings"
// Camera focal length
#define KEY_FOCAL "FocalLength"

// SDK settings section
#define SECTION_SDKSET "SDKSettings"
// Whether or not tracking is enabled
#define KEY_TRACKING "TrackingEnabled"
// The minimum confidence interval
#define KEY_CONF "ConfidenceMin"
// Use CUDA graphs
#define KEY_USE_CUDA "UseCudaGraph"
// Stabilization
#define KEY_STABLE "Stabilization"
// Batch size
#define KEY_BATCH_SZ "BatchSize"
// NV AR mode
#define KEY_NVAR "NVARMode"
// Image width dimensions
#define KEY_IMAGE_W "ImageWidth"
// Image height dimensions
#define KEY_IMAGE_H "ImageHeight"

// Zero
#define C_0 "0"

class CVirtualBodyTracker;
class CVirtualBaseStation;

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    static const char* const msInterfaces[];  

    vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
    void Cleanup();
    const char* const* GetInterfaceVersions();
    void RunFrame();
    bool ShouldBlockStandbyMode();
    void EnterStandby();
    void LeaveStandby();

    bool standby;
    bool trackingEnabled;

    CBodyTrackDriver* driver;

    CServerDriver(const CServerDriver& that) = delete;
    CServerDriver& operator=(const CServerDriver& that) = delete;

    CSimpleIniA iniFile;

    char tbuffer[BUFFER_SIZE] = { NULL };

    std::vector<CVirtualBodyTracker*> trackers;
    CVirtualBaseStation* station;


    void Initialize();

    inline int GetConfigInteger(const char* section, const char* key, int def = 0) { return atoi(iniFile.GetValue(section, key, C_0)); }
    inline float GetConfigFloat(const char* section, const char* key, float def = 0.0f) { return (float)iniFile.GetDoubleValue(section, key, 0.0); }
    glm::vec3 GetConfigVector(const char* section);
    glm::quat GetConfigQuaternion(const char* section);
    inline bool GetConfigBoolean(const char* section, const char* key, bool def = false) { return iniFile.GetBoolValue(section, key, def); }
    inline const char* GetConfigString(const char* section, const char* key, const char* def = "") { return iniFile.GetValue(section, key, def); }

    inline bool SetConfigInteger(const char* section, const char* key, int value) { _itoa_s(value, tbuffer, BUFFER_SIZE); return 0 < iniFile.SetValue(section, key, tbuffer); }
    inline bool SetConfigFloat(const char* section, const char* key, float value) { return 0 < iniFile.SetDoubleValue(section, key, (double)value); }
    bool SetConfigVector(const char* section, const glm::vec3 value);
    bool SetConfigQuaternion(const char* section, const glm::quat value);
    inline bool SetConfigBoolean(const char* section, const char* key, bool value) { return 0 < iniFile.SetBoolValue(section, key, value); }
    inline bool SetConfigString(const char* section, const char* key, const char* value) { return 0 < iniFile.SetValue(section, key, value); }
public:
    CServerDriver();
    ~CServerDriver();

    void AttachConfig(bool update=true);
    bool UpdateConfig();
    bool SaveConfig(bool update=false);
    bool SaveConfig(const char* alt);
    bool LoadConfig();
};