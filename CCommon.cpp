#include "pch.h"
#include "CCommon.h"

char logging_buffer[LOG_BUFFER_SIZE] = { NULL };

const char *TrackerRoleName[] = {
   "RTX-Hips",
   "RTX-LeftFoot",
   "RTX-RightFoot",
   "RTX-LeftElbow",
   "RTX-RightElbow",
   "RTX-LeftKnee",
   "RTX-RightKnee",
   "RTX-Chest",
   "RTX-LeftShoulder",
   "RTX-RightShoulder",
   "RTX-LeftToe",
   "RTX-RightToe",
   "RTX-Head",
   "RTX-LeftHand",
   "RTX-RightHand"
};
