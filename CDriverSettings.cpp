#include "pch.h"
#include "CDriverSettings.h"

extern char g_modulePath[];

CDriverSettings::CDriverSettings()
{
    m_filePath.assign(g_modulePath);
    m_filePath.erase(m_filePath.begin() + m_filePath.rfind('\\'), m_filePath.end());
    m_filePath.append(C_SETTINGS);
}
CDriverSettings::~CDriverSettings()
{
}

bool CDriverSettings::UpdateConfig()
{
    /*
    return SetConfigVector(SECTION_POS, m_bodyTracker->GetCameraPos())
        && SetConfigQuaternion(SECTION_ROT, m_bodyTracker->GetCameraRot())
        && SetConfigFloat(SECTION_CAMSET, KEY_FOCAL, m_bodyTracker->focalLength)
        && SetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, m_bodyTracker->useCudaGraph)
        && SetConfigBoolean(SECTION_SDKSET, KEY_STABLE, m_bodyTracker->stabilization)
        && SetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, m_bodyTracker->batchSize)
        && SetConfigInteger(SECTION_SDKSET, KEY_NVAR, m_bodyTracker->nvARMode)
        && SetConfigFloat(SECTION_SDKSET, KEY_CONF, m_bodyTracker->confidenceRequirement)
        && SetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, trackingEnabled);
    */
    return true;
}

bool CDriverSettings::SaveConfig(bool update)
{
    if(update)
        UpdateConfig();
    return 0 < m_iniFile.SaveFile(m_filePath.c_str());
}

bool CDriverSettings::SaveConfig(const char *alt)
{
    return 0 < m_iniFile.SaveFile(alt);
}

bool CDriverSettings::LoadConfig()
{
    return 0 < m_iniFile.LoadFile(m_filePath.c_str());
}

glm::vec3 CDriverSettings::GetConfigVector(const char *section)
{
    return glm::vec3(
        GetConfigFloat(section, C_X),
        GetConfigFloat(section, C_Y),
        GetConfigFloat(section, C_Z)
    );
}
glm::quat CDriverSettings::GetConfigQuaternion(const char *section)
{
    return glm::quat(
        GetConfigFloat(section, C_W),
        GetConfigFloat(section, C_X),
        GetConfigFloat(section, C_Y),
        GetConfigFloat(section, C_Z)
    );
}

bool CDriverSettings::SetConfigVector(const char *section, const glm::vec3 value)
{
    return SetConfigFloat(section, C_X, value.x)
        && SetConfigFloat(section, C_Y, value.y)
        && SetConfigFloat(section, C_Z, value.z);
}

bool CDriverSettings::SetConfigQuaternion(const char *section, const glm::quat value)
{
    return SetConfigFloat(section, C_W, value.w)
        && SetConfigFloat(section, C_X, value.x)
        && SetConfigFloat(section, C_Y, value.y)
        && SetConfigFloat(section, C_Z, value.z);
}

TRACKING_FLAG CDriverSettings::GetConfigTrackingFlag(const char *section, const char *key, TRACKING_FLAG expected, TRACKING_FLAG def = TRACKING_FLAG::NONE)
{
    return GetConfigBoolean(section, key, false) ? expected : def;
}

TRACKING_FLAG CDriverSettings::GetConfigTrackingMode(const char *section, TRACKING_FLAG def)
{
    TRACKING_FLAG m_flags = GetConfigTrackingFlag(section, KEY_HIP_ON, TRACKING_FLAG::HIP)
        | GetConfigTrackingFlag(section, KEY_FEET_ON, TRACKING_FLAG::FEET)
        | GetConfigTrackingFlag(section, KEY_ELBOW_ON, TRACKING_FLAG::ELBOW)
        | GetConfigTrackingFlag(section, KEY_KNEE_ON, TRACKING_FLAG::KNEE)
        | GetConfigTrackingFlag(section, KEY_CHEST_ON, TRACKING_FLAG::CHEST)
        | GetConfigTrackingFlag(section, KEY_SHOULDER_ON, TRACKING_FLAG::SHOULDER)
        | GetConfigTrackingFlag(section, KEY_TOE_ON, TRACKING_FLAG::TOE)
        | GetConfigTrackingFlag(section, KEY_HEAD_ON, TRACKING_FLAG::HEAD)
        | GetConfigTrackingFlag(section, KEY_HAND_ON, TRACKING_FLAG::HAND);
    if(m_flags == TRACKING_FLAG::NONE)
        return def;
    else
        return m_flags;
}
