#pragma once
#include "CCommon.h"

//  The size of the buffer used when querying information from the config file
#define BUFFER_SIZE 50


//  X Coordinate of a vector/quaternion (float)
#define C_X "X"
//  Y Coordinate of a vector/quaternion (float)
#define C_Y "Y"
//  Z Coordinate of a vector/quaternion (float)
#define C_Z "Z"
//  W Coordinate of a quaternion (float)
#define C_W "W"


//  Settings file path
#define C_SETTINGS "\\settings.ini"


//  Camera position section (glm::vec3)
#define SECTION_POS "CameraPosition"
//  Camera rotation section (glm::vec3)
#define SECTION_ROT "CameraRotation"


//  Camera settings section
#define SECTION_CAMSET "CameraSettings"
//  Camera focal length (float)
#define KEY_FOCAL "FocalLength"
//  Resolution scale (float)
#define KEY_RES_SCALE "ResolutionScale"
//  Whether or not to make the camera visible (bool)
#define KEY_CAM_VIS "Visible"
//  The index of the camera to use
#define KEY_CAM_INDEX "Index"

//  Tracking scale on X, Y, and Z axis
#define SECTION_TRACK_SCALE "TrackingScale"

//  SDK settings section
#define SECTION_SDKSET "SDKSettings"
//  Whether or not tracking is enabled (bool)
#define KEY_TRACKING "TrackingEnabled"
//  The minimum m_confidence interval (float)
#define KEY_CONF "ConfidenceMin"
//  Use CUDA graphs (bool)
#define KEY_USE_CUDA "UseCudaGraph"
//  Stabilization (bool)
#define KEY_STABLE "Stabilization"
//  Batch size (int)
#define KEY_BATCH_SZ "BatchSize"
//  NV AR mode (int)
#define KEY_NVAR "NVARMode"


//  Which m_trackers are enabled currently
#define SECTION_TRACK_MODE "EnabledTrackers"
//  Hip tracking enabled (bool)
#define KEY_HIP_ON "Hips"
//  Foot tracking enabled (bool)
#define KEY_FEET_ON "Feet"
//  Elbow tracking enabled (bool)
#define KEY_ELBOW_ON "Elbows"
//  Knee tracking enabled (bool)
#define KEY_KNEE_ON "Knees"
//  Chest tracking enabled (bool)
#define KEY_CHEST_ON "Chest"
//  Shoulder tracking enabled (bool)
#define KEY_SHOULDER_ON "Shoulders"
//  Toe tracking enabled (bool)
#define KEY_TOE_ON "Toes"
//  Head tracking enabled (bool)
#define KEY_HEAD_ON "Head"
//  Hand tracking enabled (bool)
#define KEY_HAND_ON "Hand"


//  Additional settings for the trackers
#define SECTION_TRACKSET "TrackerSettings"

//  Interpolation mode to use for the trackers (One of [None, Linear, Sinusoidal, Quadratic, Cubic])
#define KEY_INTERP "Interpolation"
//  No interpolation applied
#define INTERP_NONE "None"
//  Linear interpolation applied
#define INTERP_LIN "Linear"
//  Sinusoidal interpolation applied
#define INTERP_SINE "Sinusoidal"
//  Quadratic interpolation applied
#define INTERP_QUAD "Quadratic"
//  Cubic interpolation applied
#define INTERP_CUBE "Cubic"

//  The offset of the elbow trackers
#define KEY_ELBOW_POS "ElbowTrackerPosition"
//  The offset of the knee trackers
#define KEY_KNEE_POS "KneeTrackerPosition"
//  The offset of the hip trackers
#define KEY_HIP_POS "HipTrackerPosition"
//  The offset of the chest trackers
#define KEY_CHEST_POS "ChestTrackerPosition"


//  Zero
#define C_0 "0"

class CServerDriver;

enum class TRACKING_FLAG;
enum class TRACKER_ROLE;

//  The interpolation mode for the tracker
enum class INTERP_MODE
{
    NONE,
    LINEAR,
    SINE,
    QUAD,
    CUBIC
};
const char *InterpModeName[];

//  Used to store the proportional information from the config file
struct Proportions
{
    float
        elbowOffset,
        kneeOffset,
        hipOffset,
        chestOffset;

    Proportions(float hip = 0.0f, float elbow = 0.0f, float knee = 0.0f, float chest = 0.0f) 
        : hipOffset(hip), elbowOffset(elbow), kneeOffset(knee), chestOffset(chest) {}
};

/// <summary>
/// Responsible for reading, managing, and storing information from the <b>settings.ini</b> configuration file
/// </summary>
class CDriverSettings
{
    std::string m_filePath;
    CSimpleIniA m_iniFile;
    char m_tempBuffer[BUFFER_SIZE] = { NULL };
public:
    CDriverSettings();
    ~CDriverSettings();

    inline int GetConfigInteger(const char *section, const char *key, int def = 0) const { return atoi(m_iniFile.GetValue(section, key, C_0)); }
    inline float GetConfigFloat(const char *section, const char *key, float def = 0.0f) const { return (float)m_iniFile.GetDoubleValue(section, key, 0.0); }
    glm::vec3 GetConfigVector(const char *section, const glm::vec3 &def=glm::vec3(0.f, 0.f, 0.f)) const;
    glm::quat GetConfigQuaternion(const char *section, const glm::quat &def = glm::quat(0.f, 0.f, 0.f, 0.f)) const;
    inline  bool GetConfigBoolean(const char *section, const char *key, bool def = false) const { return m_iniFile.GetBoolValue(section, key, def); }
    inline const char *GetConfigCString(const char *section, const char *key, const char *def = "") const { return m_iniFile.GetValue(section, key, def); }
    inline const std::string GetConfigString(const char *section, const char *key, const std::string &def = std::string("")) const { return std::string(m_iniFile.GetValue(section, key, def.c_str())); }

    inline bool SetConfigInteger(const char *section, const char *key, int value) { _itoa_s(value, m_tempBuffer, BUFFER_SIZE); return 0 < m_iniFile.SetValue(section, key, m_tempBuffer); }
    inline  bool SetConfigFloat(const char *section, const char *key, float value) { return 0 < m_iniFile.SetDoubleValue(section, key, (double)value); }
    bool SetConfigVector(const char *section, const glm::vec3 value);
    bool SetConfigQuaternion(const char *section, const glm::quat value);
    inline bool SetConfigBoolean(const char *section, const char *key, bool value) { return 0 < m_iniFile.SetBoolValue(section, key, value); }
    inline bool SetConfigCString(const char *section, const char *key, const char *value) { return 0 < m_iniFile.SetValue(section, key, value); }
    inline bool SetConfigString(const char *section, const char *key, const std::string &value) { return 0 < m_iniFile.SetValue(section, key, value.c_str()); }

    TRACKING_FLAG GetConfigTrackingFlag(const char *section, const char *key, TRACKING_FLAG expected, TRACKING_FLAG def = TRACKING_FLAG::NONE) const;
    TRACKING_FLAG GetConfigTrackingMode(const char *section, TRACKING_FLAG def = TRACKING_FLAG::NONE) const;

    INTERP_MODE GetConfigInterpolationMode(const char *section, const char *key, INTERP_MODE def = INTERP_MODE::NONE) const;
    const Proportions GetConfigProportions(const char *section, const Proportions &def = Proportions()) const;

    /// <summary>
    /// Update the configuration data with information from a source <b>CServerDriver</b>
    /// </summary>
    bool UpdateConfig(CServerDriver *source);
    bool SaveConfig();
    bool SaveConfig(const char *alt);
    bool LoadConfig();
};

