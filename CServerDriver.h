#pragma once

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

    CServerDriver(const CServerDriver& that) = delete;
    CServerDriver& operator=(const CServerDriver& that) = delete;
public:
    CServerDriver();
    ~CServerDriver();
};