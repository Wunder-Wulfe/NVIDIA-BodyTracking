#include "pch.h"
#include "CVirtualBodyTracker.h"
#include "Common.h"
#include "CServerDriver.h"

CVirtualBodyTracker::CVirtualBodyTracker(CServerDriver* driv, size_t p_index, TRACKER_ROLE rle) : CVirtualDevice(driv)
{
    m_serial.assign(TrackerRoleName[(int)rle]);
    m_index = p_index;
    role = rle;
}

CVirtualBodyTracker::~CVirtualBodyTracker()
{
}

void CVirtualBodyTracker::SetupProperties()
{
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingSystemName_String, "lighthouse");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModelNumber_String, "RTX Tracker");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_SerialNumber_String, m_serial.c_str()); // Changed
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_WillDriftInYaw_Bool, false);
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ManufacturerName_String, "HTC");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingFirmwareVersion_String, "1541800000 RUNNER-WATCHMAN$runner-watchman@runner-watchman 2018-01-01 FPGA 512(2.56/0/0) BL 0 VRC 1541800000 Radio 1518800000"); // Changed
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_HardwareRevision_String, "product 128 rev 2.5.6 lot 2000/0/0 0"); // Changed
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ConnectedWirelessDongle_String, "D0000BE000"); // Changed
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceIsWireless_Bool, true);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceIsCharging_Bool, false);
    vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

    vr::HmdMatrix34_t l_transform = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
    vr::VRProperties()->SetProperty(m_propertyHandle, vr::Prop_StatusDisplayTransform_Matrix34, &l_transform, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_UpdateAvailable_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_ManualUpdate_Bool, false);
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_HardwareRevision_Uint64, 2214720000); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_FirmwareVersion_Uint64, 1541800000); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_FPGAVersion_Uint64, 512); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_VRCVersion_Uint64, 1514800000); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_RadioVersion_Uint64, 1518800000); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_DongleVersion_Uint64, 8933539758); // Changed, based on vr::Prop_ConnectedWirelessDongle_String above
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceCanPowerOff_Bool, true);
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_Firmware_ProgrammingTarget_String, m_serial.c_str());
    vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
    //vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_ParentDriver_Uint64, 8589934597); // Strange value from dump
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ResourceRoot_String, "htc");

    std::string l_registeredType("htc/vive_tracker");
    l_registeredType.append(m_serial);
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RegisteredDeviceType_String, l_registeredType.c_str());

    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json");
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Identifiable_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_RemindUpdate_Bool, false);
    vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_Invalid);

    switch (role)
    {

    case TRACKER_ROLE::HEAD:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_camera");
        break;

    case TRACKER_ROLE::HIPS:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_waist");
        break;

    case TRACKER_ROLE::CHEST:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_chest");
        break;

    case TRACKER_ROLE::LEFT_SHOULDER:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_left_shoulder");
        break;

    case TRACKER_ROLE::RIGHT_SHOULDER:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_right_shoulder");
        break;

    case TRACKER_ROLE::LEFT_ELBOW:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_left_elbow");
        break;
    case TRACKER_ROLE::RIGHT_ELBOW:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_right_elbow");
        break;

    case TRACKER_ROLE::LEFT_HAND:
    case TRACKER_ROLE::RIGHT_HAND:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_handed");
        break;

    case TRACKER_ROLE::LEFT_KNEE:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_left_knee");
        break;

    case TRACKER_ROLE::RIGHT_KNEE:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_right_knee");
        break;

    case TRACKER_ROLE::LEFT_TOE:
    case TRACKER_ROLE::LEFT_FOOT:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_left_foot");
        break;

    case TRACKER_ROLE::RIGHT_TOE:
    case TRACKER_ROLE::RIGHT_FOOT:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_right_foot");
        break;

    default:
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ControllerType_String, "vive_tracker_none");
        break;
    }

    vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_ControllerHandSelectionPriority_Int32, -1);
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceOff_String, "{nvidiaBodyTracking}/icons/tracker/tracker_status_off.png");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/tracker_status_searching.gif");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/tracker_status_searching_alert.gif");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReady_String, "{nvidiaBodyTracking}/icons/tracker/tracker_status_ready.png");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/tracker_status_ready_alert.png");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceNotReady_String, "{nvidiaBodyTracking}/icons/tracker/tracker_status_error.png");
    vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceStandby_String, "{nvidiaBodyTracking}/icons/tracker/tracker_status_standby.png");
    //vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/tracker_status_ready_low.png");
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDisplayComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasCameraComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasVirtualDisplayComponent_Bool, false);
}