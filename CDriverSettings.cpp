#include "pch.h"
#include "CDriverSettings.h"
#include "CServerDriver.h"
#include "CNvSDKInterface.h"
#include "CCameraDriver.h"

extern char g_modulePath[];

const char *InterpModeName[] = {
    INTERP_NONE,
    INTERP_LIN,
    INTERP_SINE,
    INTERP_QUAD,
    INTERP_CUBE
};

CDriverSettings::CDriverSettings()
{
    m_filePath.assign(g_modulePath);
    m_filePath.erase(m_filePath.begin() + m_filePath.rfind("\\bin"), m_filePath.end());
    m_filePath.append(C_SETTINGS);
}
CDriverSettings::~CDriverSettings()
{
}

bool CDriverSettings::UpdateConfig(const CServerDriver *source)
{
    return SetConfigVector(SECTION_POS, source->m_nvInterface->GetCameraPos())
        && SetConfigVector(SECTION_ROT, source->m_camBryan)
        && SetConfigVector(SECTION_TRACK_SCALE, source->m_scaleFactor);
        //&& SetConfigInteger(SECTION_CAMSET, KEY_CAM_INDEX, source->m_cameraDriver->GetIndex());
}

bool CDriverSettings::SaveConfig()
{
    return 0 == m_iniFile.SaveFile(m_filePath.c_str());
}

bool CDriverSettings::SaveConfig(const char *alt)
{
    return 0 == m_iniFile.SaveFile(alt);
}

bool CDriverSettings::LoadConfig()
{
    return 0 == m_iniFile.LoadFile(m_filePath.c_str());
}

glm::vec3 CDriverSettings::GetConfigVector(const char *section, const glm::vec3 &def) const
{
    return glm::vec3(
        GetConfigFloat(section, C_X, def.x),
        GetConfigFloat(section, C_Y, def.y),
        GetConfigFloat(section, C_Z, def.z)
    );
}
glm::quat CDriverSettings::GetConfigQuaternion(const char *section, const glm::quat &def) const
{
    return glm::quat(
        GetConfigFloat(section, C_W, def.w),
        GetConfigFloat(section, C_X, def.x),
        GetConfigFloat(section, C_Y, def.y),
        GetConfigFloat(section, C_Z, def.z)
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

TRACKING_FLAG CDriverSettings::GetConfigTrackingFlag(const char *section, const char *key, TRACKING_FLAG expected, TRACKING_FLAG def) const
{
    return GetConfigBoolean(section, key, false) ? expected : def;
}

TRACKING_FLAG CDriverSettings::GetConfigTrackingMode(const char *section, TRACKING_FLAG def) const
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

INTERP_MODE CDriverSettings::GetConfigInterpolationMode(const char *section, const char *key, INTERP_MODE def) const
{
    std::string result = GetConfigString(section, key, INTERP_NONE);
    if (result == INTERP_CUBE)
    {
        return INTERP_MODE::CUBIC;
    }
    else if (result == INTERP_QUAD)
    {
        return INTERP_MODE::QUAD;
    }
    else if (result == INTERP_SINE)
    {
        return INTERP_MODE::SINE;
    }
    else if (result == INTERP_LIN)
    {
        return INTERP_MODE::LINEAR;
    }
    else
    {
        return def;
    }
}

const Proportions CDriverSettings::GetConfigProportions(const char *section, const Proportions &def) const
{
    Proportions result;
    result.hipOffset    = GetConfigFloat(section, KEY_HIP_POS, def.hipOffset);
    result.elbowOffset  = GetConfigFloat(section, KEY_ELBOW_POS, def.elbowOffset);
    result.kneeOffset   = GetConfigFloat(section, KEY_KNEE_POS, def.kneeOffset);
    result.chestOffset  = GetConfigFloat(section, KEY_CHEST_POS, def.chestOffset);
    result.footOffset   = GetConfigFloat(section, KEY_FOOT_POS, def.footOffset);
    return result;
}