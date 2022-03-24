#include "pch.h"

#include "CServerDriver.h"


const char* const CServerDriver::msInterfaces[]
{
	vr::ITrackedDeviceServerDriver_Version,
	vr::IServerTrackedDeviceProvider_Version,
	nullptr
};

CServerDriver::CServerDriver()
{
	driver = new CBodyTrackDriver();
}

CServerDriver::~CServerDriver()
{
	Cleanup();
}

bool CServerDriver::UpdateConfig()
{
	return SetConfigVector(SECTION_POS, driver->GetCameraPos())
		&& SetConfigQuaternion(SECTION_ROT, driver->GetCameraRot())
		&& SetConfigFloat(SECTION_CAMSET, KEY_FOCAL, driver->focalLength)
		&& SetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, driver->useCudaGraph)
		&& SetConfigBoolean(SECTION_SDKSET, KEY_STABLE, driver->stabilization)
		&& SetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, driver->batchSize)
		&& SetConfigInteger(SECTION_SDKSET, KEY_NVAR, driver->nvARMode)
		&& SetConfigInteger(SECTION_SDKSET, KEY_IMAGE_W, driver->ImageWidth())
		&& SetConfigInteger(SECTION_SDKSET, KEY_IMAGE_H, driver->ImageHeight());
}

bool CServerDriver::SaveConfig(bool update=false)
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
	return 0 < iniFile.LoadFile(C_SETTINGS);
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
	return SetConfigFloat(section, C_X, value.x)
		&& SetConfigFloat(section, C_Y, value.y)
		&& SetConfigFloat(section, C_Z, value.z)
		&& SetConfigFloat(section, C_W, value.w);
}

void CServerDriver::Initialize()
{
	LoadConfig();

	AttachConfig(false);

	driver->Initialize();
}

void CServerDriver::AttachConfig(bool update=true)
{
	driver->batchSize = GetConfigInteger(SECTION_SDKSET, KEY_BATCH_SZ, 1);
	driver->focalLength = GetConfigFloat(SECTION_CAMSET, KEY_FOCAL, 800.0f);
	driver->stabilization = GetConfigBoolean(SECTION_SDKSET, KEY_STABLE, true);
	driver->useCudaGraph = GetConfigBoolean(SECTION_SDKSET, KEY_USE_CUDA, true);
	driver->nvARMode = GetConfigInteger(SECTION_SDKSET, KEY_NVAR, 1);
	driver->ResizeImage(
		GetConfigInteger(SECTION_SDKSET, KEY_IMAGE_W, 1920),
		GetConfigInteger(SECTION_SDKSET, KEY_IMAGE_H, 1080)
	);

	driver->SetCamera(
		GetConfigVector(SECTION_POS),
		GetConfigQuaternion(SECTION_ROT)
	);

	if (update)
		driver->KeyInfoUpdated();
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	Initialize();

	return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
	delete driver;
	driver = nullptr;
	vr::CleanupDriverContext();
}

void CServerDriver::RunFrame()
{
}

void CServerDriver::EnterStandby()
{
}

void CServerDriver::LeaveStandby()
{
}

bool CServerDriver::ShouldBlockStandbyMode()
{
	return false;
}

const char* const* CServerDriver::GetInterfaceVersions()
{
	return msInterfaces;
}
