#include "pch.h"
#include "CCommon.h"

char logging_buffer[LOG_BUFFER_SIZE] = { NULL };

const char *BodyJointName[] = {
    "Pelvis",
    "Left Hip",
    "Right Hip",
    "Torso",
    "Left Knee",
    "Right Knee",
    "Neck",
    "Left Ankle",
    "Right Ankle",
    "Left Big Toe",
    "Right Big Toe",
    "Left Small Toe",
    "Right Small Toe",
    "Left Heel",
    "Right Heel",
    "Nose",
    "Left Eye",
    "Right Eye",
    "Left Ear",
    "Right Ear",
    "Left Shoulder",
    "Right Shoulder",
    "Left Elbow",
    "Right Elbow",
    "Left Wrist",
    "Right Wrist",
    "Left Pinky Finger",
    "Right Pinky Finger",
    "Left Middle Finger",
    "Right Middle Finger",
    "Left Index Finger",
    "Right Index Finger",
    "Left Thumb",
    "Right Thumb"
};

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