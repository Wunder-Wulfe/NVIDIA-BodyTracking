#include "pch.h"
#include "CVirtualBaseStation.h"
#include "CServerDriver.h"

CVirtualBaseStation::CVirtualBaseStation(CServerDriver *p_server)
{
    m_serial.assign("RTX-30");
    m_serverDriver = p_server;
}

CVirtualBaseStation::~CVirtualBaseStation()
{
}

void CVirtualBaseStation::SetupProperties()
{
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingSystemName_String, "rtxstation");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_SerialNumber_String, m_serial.c_str());
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModelNumber_String, "RTX Tracker Driver");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ManufacturerName_String, "NVIDIA");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModeLabel_String, "K");

    vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_TrackingReference);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_IsOnDesktop_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_NeverTracked_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_WillDriftInYaw_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_CanWirelessIdentify_Bool, false);

    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewLeftDegrees_Float, 70.f);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewRightDegrees_Float, 70.f);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewTopDegrees_Float, 60.f);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewBottomDegrees_Float, 60.f);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_TrackingRangeMinimumMeters_Float, 0.5f);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_TrackingRangeMaximumMeters_Float, 4.5f);

    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ResourceRoot_String, "nvidiaBodyTracking");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RenderModelName_String, "{nvidiaBodyTracking}/rendermodels/base/nvidiaStation");

    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceOff_String, "{nvidiaBodyTracking}/icons/base/base_status_off.png");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearching_String, "{nvidiaBodyTracking}/icons/base_status_searching.gif");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{nvidiaBodyTracking}/icons/base_status_searching_alert.gif");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReady_String, "{nvidiaBodyTracking}/icons/base/base_status_ready.png");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{nvidiaBodyTracking}/icons/base_status_ready_alert.png");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceNotReady_String, "{nvidiaBodyTracking}/icons/base/base_status_error.png");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceStandby_String, "{nvidiaBodyTracking}/icons/base/base_status_standby.png");

    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDisplayComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasCameraComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasVirtualDisplayComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_BlockServerShutdown_Bool, false);

    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_VendorSpecific_Reserved_Start, 0x525458547261636B); // "RTXTrack"
}

void CVirtualBaseStation::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
    //m_serverDriver->ProcessExternalMessage(pchRequest);
}