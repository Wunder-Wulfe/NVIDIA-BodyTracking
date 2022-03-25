#include "pch.h"

#include "CServerDriver.h"
#include "CNvBodyTracker.h"
#include "CVirtualBaseStation.h"
#include "CCameraDriver.h"
#include "CCommon.h"

template<class T>
inline void delptr(T& ptr) {
	delete ptr;
	ptr = nullptr;
}

extern char g_modulePath[];

const char* const CServerDriver::msInterfaces[]
{
	vr::ITrackedDeviceServerDriver_Version,
	vr::IServerTrackedDeviceProvider_Version,
	nullptr
};

CServerDriver::CServerDriver()
{
	driver = nullptr;
	camDriver = nullptr;
	station = nullptr;
	standby = false;
	trackingEnabled = false;
}

CServerDriver::~CServerDriver()
{
	Cleanup();
}


bool CServerDriver::UpdateConfig()
{
	/*
	return SetConfigVector(SECTION_POS, driver->GetCameraPos())
		&& SetConfigQuaternion(SECTION_ROT, driver->GetCameraRot())
		&& SetConfigFloat(SECTION_CAMSET, KEY_FOCAL, driver->focalLength)
		&& SetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, driver->useCudaGraph)
		&& SetConfigBoolean(SECTION_SDKSET, KEY_STABLE, driver->stabilization)
		&& SetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, driver->batchSize)
		&& SetConfigInteger(SECTION_SDKSET, KEY_NVAR, driver->nvARMode)
		&& SetConfigFloat(SECTION_SDKSET, KEY_CONF, driver->confidenceRequirement)
		&& SetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, trackingEnabled);
	*/
	return true;
}

bool CServerDriver::SaveConfig(bool update)
{
	if (update)
		UpdateConfig();
	return 0 < iniFile.SaveFile(C_SETTINGS);
}

bool CServerDriver::SaveConfig(const char* alt)
{
	return 0 < iniFile.SaveFile(alt);
}

bool CServerDriver::LoadConfig()
{
	std::string l_path(g_modulePath);
	l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
	l_path.append(C_SETTINGS);
	return 0 < iniFile.LoadFile(l_path.c_str());
}

glm::vec3 CServerDriver::GetConfigVector(const char* section)
{
	return glm::vec3(
		GetConfigFloat(section, C_X),
		GetConfigFloat(section, C_Y),
		GetConfigFloat(section, C_Z)
	);
}
glm::quat CServerDriver::GetConfigQuaternion(const char* section)
{
	return glm::quat(
		GetConfigFloat(section, C_W),
		GetConfigFloat(section, C_X),
		GetConfigFloat(section, C_Y),
		GetConfigFloat(section, C_Z)
	);
}

bool CServerDriver::SetConfigVector(const char* section, const glm::vec3 value)
{
	return SetConfigFloat(section, C_X, value.x)
			&& SetConfigFloat(section, C_Y, value.y)
			&& SetConfigFloat(section, C_Z, value.z);
}

bool CServerDriver::SetConfigQuaternion(const char* section, const glm::quat value)
{
	return SetConfigFloat(section, C_W, value.w)
		&& SetConfigFloat(section, C_X, value.x)
		&& SetConfigFloat(section, C_Y, value.y)
		&& SetConfigFloat(section, C_Z, value.z);
}

TRACKING_FLAG CServerDriver::GetConfigTrackingFlag(const char* section, const char* key, TRACKING_FLAG expected, TRACKING_FLAG def = TRACKING_FLAG::NONE)
{
	return GetConfigBoolean(section, key, false) ? expected : def;
}

TRACKING_FLAG CServerDriver::GetConfigTrackingMode(const char* section, TRACKING_FLAG def = TRACKING_FLAG::NONE)
{
	TRACKING_FLAG flags = GetConfigTrackingFlag(section, KEY_HIP_ON, TRACKING_FLAG::HIP)
		| GetConfigTrackingFlag(section, KEY_FEET_ON, TRACKING_FLAG::FEET)
		| GetConfigTrackingFlag(section, KEY_ELBOW_ON, TRACKING_FLAG::ELBOW)
		| GetConfigTrackingFlag(section, KEY_KNEE_ON, TRACKING_FLAG::KNEE)
		| GetConfigTrackingFlag(section, KEY_CHEST_ON, TRACKING_FLAG::CHEST)
		| GetConfigTrackingFlag(section, KEY_SHOULDER_ON, TRACKING_FLAG::SHOULDER)
		| GetConfigTrackingFlag(section, KEY_TOE_ON, TRACKING_FLAG::TOE)
		| GetConfigTrackingFlag(section, KEY_HEAD_ON, TRACKING_FLAG::HEAD)
		| GetConfigTrackingFlag(section, KEY_HAND_ON, TRACKING_FLAG::HAND);
	if (flags == TRACKING_FLAG::NONE)
		return def;
	else
		return flags;
}

void CServerDriver::Initialize()
{
	driver = new CNvBodyTracker();
	station = new CVirtualBaseStation(this);
	camDriver = new CCameraDriver(GetConfigFloat(SECTION_CAMSET, KEY_RES_SCALE, 1.0));
	camDriver->show = GetConfigBoolean(SECTION_CAMSET, KEY_CAM_VIS, true);

	trackingMode = GetConfigTrackingMode(SECTION_TRACK_MODE);

	LoadConfig();

	AttachConfig(false);

	camDriver->LoadCameras();

	driver->Initialize();
}

void CServerDriver::AttachConfig(bool update)
{
	driver->batchSize = GetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, 1);
	driver->focalLength = GetConfigFloat(SECTION_CAMSET, KEY_FOCAL, 800.0f);
	driver->stabilization = GetConfigBoolean(SECTION_SDKSET, KEY_STABLE, true);
	driver->useCudaGraph = GetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, true);
	driver->nvARMode = GetConfigInteger(SECTION_SDKSET, KEY_NVAR, 1);

	driver->SetCamera(
		GetConfigVector(SECTION_POS),
		GetConfigQuaternion(SECTION_ROT)
	);
	driver->confidenceRequirement = GetConfigFloat(SECTION_SDKSET, KEY_CONF, 0.0);

	driver->trackingActive = GetConfigBoolean(SECTION_SDKSET, KEY_TRACKING, false);

	if (update)
		driver->KeyInfoUpdated();
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	Initialize();

	station->AddTracker();

	return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
	delptr(driver);
	delptr(camDriver);
	delptr(station);

	vr::CleanupDriverContext();
}

void CServerDriver::RunFrame()
{
	driver->trackingActive = trackingEnabled && !standby;
	camDriver->RunFrame();
	driver->RunFrame();
	station->SetConnected(driver->trackingActive && driver->GetConfidenceAcceptable());
	station->RunFrame();
}

void CServerDriver::EnterStandby()
{
	standby = true;
}

void CServerDriver::LeaveStandby()
{
	standby = false;
}

bool CServerDriver::ShouldBlockStandbyMode()
{
	return false;
}

const char* const* CServerDriver::GetInterfaceVersions()
{
	return msInterfaces;
}
