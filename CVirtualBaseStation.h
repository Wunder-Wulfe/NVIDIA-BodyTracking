#pragma once

#include "CVirtualDevice.h"

class CServerDriver;

class CVirtualBaseStation : public CVirtualDevice
{
    CServerDriver *m_serverDriver;

    CVirtualBaseStation(const CVirtualBaseStation &that) = delete;
    CVirtualBaseStation &operator=(const CVirtualBaseStation &that) = delete;

    void SetupProperties() override;

    // vr::ITrackedDeviceServerDriver
    void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;
public:
    explicit CVirtualBaseStation(CServerDriver *p_server);
    ~CVirtualBaseStation();
};

