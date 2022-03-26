#pragma once
#include "CCommon.h"

#define BUFFER_SIZE 20

// X Coordinate of a vector/quaternion (float)
#define C_X "X"
// Y Coordinate of a vector/quaternion (float)
#define C_Y "Y"
// Z Coordinate of a vector/quaternion (float)
#define C_Z "Z"
// W Coordinate of a quaternion (float)
#define C_W "W"

// Settings file path
#define C_SETTINGS "\\settings.ini"

// Camera position section (glm::vec3)
#define SECTION_POS "CameraPosition"
// Camera rotation section (glm::quat)
#define SECTION_ROT "CameraRotation"

// Camera settings section
#define SECTION_CAMSET "CameraSettings"
// Camera focal length (float)
#define KEY_FOCAL "FocalLength"
// Resolution scale (float)
#define KEY_RES_SCALE "ResolutionScale"
// Whether or not to make the camera visible (bool)
#define KEY_CAM_VIS "Visible"

// SDK settings section
#define SECTION_SDKSET "SDKSettings"
// Whether or not tracking is enabled (bool)
#define KEY_TRACKING "TrackingEnabled"
// The minimum m_confidence interval (float)
#define KEY_CONF "ConfidenceMin"
// Use CUDA graphs (bool)
#define KEY_USE_CUDA "UseCudaGraph"
// Stabilization (bool)
#define KEY_STABLE "Stabilization"
// Batch size (int)
#define KEY_BATCH_SZ "BatchSize"
// NV AR mode (int)
#define KEY_NVAR "NVARMode"

// Which m_trackers are enabled currently
#define SECTION_TRACK_MODE "EnabledTrackers"
// Hip tracking enabled (bool)
#define KEY_HIP_ON "Hips"
// Foot tracking enabled (bool)
#define KEY_FEET_ON "Feet"
// Elbow tracking enabled (bool)
#define KEY_ELBOW_ON "Elbows"
// Knee tracking enabled (bool)
#define KEY_KNEE_ON "Knees"
// Chest tracking enabled (bool)
#define KEY_CHEST_ON "Chest"
// Shoulder tracking enabled (bool)
#define KEY_SHOULDER_ON "Shoulders"
// Toe tracking enabled (bool)
#define KEY_TOE_ON "Toes"
// Head tracking enabled (bool)
#define KEY_HEAD_ON "Head"
// Hand tracking enabled (bool)
#define KEY_HAND_ON "Hand"

// Zero
#define C_0 "0"

enum class TRACKING_FLAG;
enum class TRACKER_ROLE;

class CDriverSettings
{
    std::string m_filePath;
    CSimpleIniA m_iniFile;
    char m_tempBuffer[BUFFER_SIZE] = { NULL };
public:
    CDriverSettings();
    ~CDriverSettings();

    inline int GetConfigInteger(const char *section, const char *key, int def = 0) { return atoi(m_iniFile.GetValue(section, key, C_0)); }
    inline float GetConfigFloat(const char *section, const char *key, float def = 0.0f) { return (float)m_iniFile.GetDoubleValue(section, key, 0.0); }
    glm::vec3 GetConfigVector(const char *section);
    glm::quat GetConfigQuaternion(const char *section);
    inline bool GetConfigBoolean(const char *section, const char *key, bool def = false) { return m_iniFile.GetBoolValue(section, key, def); }
    inline const char *GetConfigString(const char *section, const char *key, const char *def = "") { return m_iniFile.GetValue(section, key, def); }

    inline bool SetConfigInteger(const char *section, const char *key, int value) { _itoa_s(value, m_tempBuffer, BUFFER_SIZE); return 0 < m_iniFile.SetValue(section, key, m_tempBuffer); }
    inline bool SetConfigFloat(const char *section, const char *key, float value) { return 0 < m_iniFile.SetDoubleValue(section, key, (double)value); }
    bool SetConfigVector(const char *section, const glm::vec3 value);
    bool SetConfigQuaternion(const char *section, const glm::quat value);
    inline bool SetConfigBoolean(const char *section, const char *key, bool value) { return 0 < m_iniFile.SetBoolValue(section, key, value); }
    inline bool SetConfigString(const char *section, const char *key, const char *value) { return 0 < m_iniFile.SetValue(section, key, value); }

    TRACKING_FLAG GetConfigTrackingFlag(const char *section, const char *key, TRACKING_FLAG expected, TRACKING_FLAG def);
    TRACKING_FLAG GetConfigTrackingMode(const char *section, TRACKING_FLAG def = TRACKING_FLAG::NONE);

    bool UpdateConfig();
    bool SaveConfig(bool update = false);
    bool SaveConfig(const char *alt);
    bool LoadConfig();
};

