#pragma once

#define LOG_BUFFER_SIZE 1000

#define vr_log(fmt, ...) {sprintf_s(logging_buffer, LOG_BUFFER_SIZE, fmt, __VA_ARGS__); vr::VRDriverLog()->Log(logging_buffer);}

#define delptr(ptr) {delete ptr; ptr = nullptr;}

char logging_buffer[];

enum class BODY_JOINT
{
    PELVIS,
    LEFT_HIP,
    RIGHT_HIP,
    TORSO,
    LEFT_KNEE,
    RIGHT_KNEE,
    NECK,
    LEFT_ANKLE,
    RIGHT_ANKLE,
    LEFT_BIG_TOE,
    RIGHT_BIG_TOE,
    LEFT_SMALL_TOE,
    RIGHT_SMALL_TOE,
    LEFT_HEEL,
    RIGHT_HEEL,
    NOSE,
    LEFT_EYE,
    RIGHT_EYE,
    LEFT_EAR,
    RIGHT_EAR,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    LEFT_ELBOW,
    RIGHT_ELBOW,
    LEFT_WRIST,
    RIGHT_WRIST,
    LEFT_PINKY_KNUCKLE,
    RIGHT_PINKY_KNUCKLE,
    LEFT_MIDDLE_TIP,
    RIGHT_MIDDLE_TIP,
    LEFT_INDEX_KNUCKLE,
    RIGHT_INDEX_KNUCKLE,
    LEFT_THUMB_TIP,
    RIGHT_THUMB_TIP
};

enum class TRACKER_ROLE
{
    HIPS,
    LEFT_FOOT,
    RIGHT_FOOT,
    LEFT_ELBOW,
    RIGHT_ELBOW,
    LEFT_KNEE,
    RIGHT_KNEE,
    CHEST,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    LEFT_TOE,
    RIGHT_TOE,
    HEAD,
    LEFT_HAND,
    RIGHT_HAND
};
const char *TrackerRoleName[];

enum class TRACKING_FLAG
{
    NONE = 0b0,
    HIP = 0b1,
    FEET = 0b10,
    ELBOW = 0b100,
    KNEE = 0b1000,
    CHEST = 0b10000,
    SHOULDER = 0b100000,
    TOE = 0b1000000,
    HEAD = 0b10000000,
    HAND = 0b100000000
};

inline TRACKING_FLAG operator|(const TRACKING_FLAG &a, const TRACKING_FLAG &b) { return (TRACKING_FLAG)((int)a | (int)b); }
inline TRACKING_FLAG operator&(const TRACKING_FLAG &a, const TRACKING_FLAG &b) { return (TRACKING_FLAG)((int)a & (int)b); }