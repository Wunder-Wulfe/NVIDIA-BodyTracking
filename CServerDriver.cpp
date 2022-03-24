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
}

CServerDriver::~CServerDriver()
{
	Cleanup();
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
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

