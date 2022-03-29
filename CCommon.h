#pragma once

#define M_PI 3.14159265358979323846f

#define systime() ((double)clock() / CLOCKS_PER_SEC)

//  The maximum size for our log buffer
#define LOG_BUFFER_SIZE 1000

//  General logging buffer used to report to vrserver.txt
char logging_buffer[];

//  Shorthand for logging to vrserver.txt
template<class... T>
inline void vr_log(const char *fmt, const T&... args)
{
    sprintf_s(logging_buffer, LOG_BUFFER_SIZE, fmt, args...);
    vr::VRDriverLog()->Log(logging_buffer);
}

//  Delete a pointer safely
template<class T>
inline void delptr(T &ptr)
{
    if (ptr != nullptr) delete ptr;
    ptr = nullptr;
}

//  All possible joints capable of being tracked via the NVIDIA AR SDK, used to index the keypoint tables
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
//  The name of the associated body joints
const char *BodyJointName[];

//  The roles body trackers will be able to play
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
//  The name of the associated roles
const char *TrackerRoleName[];

//  Flags used to indicate which tracking modes are applicable / enabled
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

template<class T>
inline const T FLAG_OR(const T &last) { return last; }
template<class T, class... Args>
inline const T FLAG_OR(const T &first, const Args&... rest) { return (T)((uint)first | (uint)FLAG_OR(rest...)); }

template<class T>
inline const T FLAG_AND(const T &last) { return last; }
template<class T, class... Args>
inline const T FLAG_AND(const T &first, const Args&... rest) { return (T)((uint)first & (uint)FLAG_AND(rest...)); }

template<class T>
inline const bool FLAG_ACTIVE(const T &flag, const T &last) { return (uint)FLAG_AND(flag, last) > 0; }
template<class T, class... Args>
inline const bool FLAG_ACTIVE(const T &flag, const T &first, const Args&... rest) { return ((uint)FLAG_AND(flag, first) > 0) && FLAG_ACTIVE(flag, rest...); }