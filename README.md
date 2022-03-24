# NVIDIA BodyTracking SteamVR Driver

![Powered By: NVIDIA RTX](https://user-images.githubusercontent.com/29297318/159997280-2131e876-42bd-4c8b-9472-7c88c6c7ba60.png)

This is a **WIP** driver designed to use the NVIDIA AR SDK for camera-based body tracking.
For now, most of the code is based on the [AR SDK examples](https://github.com/NVIDIA/MAXINE-AR-SDK), [Mediapipe-VR-Fullbody-Tracking](https://github.com/ju1ce/Mediapipe-VR-Fullbody-Tracking), and [driver_kinectV2](https://github.com/SDraw/driver_kinectV2/blob/master/driver_kinectV2)

[Discord Server](https://discord.gg/XjkyuwRW6Z)


Current status:

* Made config file
* Created some of the classes
* Loads and closes itself

TODO:

* Capture image data from a camera the user can select (So it actually tracks something)
* Create an overlay app/system to control the driver
* Draw more icons for SteamVR

In order to use any of the executables, you are required to download and install the [NVIDIA Broadcast AR SDK](https://www.nvidia.com/en-us/geforce/broadcasting/broadcast-sdk/resources/).
*(Note: As of right now, it works only on RTX 20 and 30 series cards. Other cards are untested)*


Additional libraries used:

1. [OpenCV](https://github.com/opencv/opencv) for image/video capture
2. [SimpleIni](https://github.com/brofield/simpleini) for the configuration file
3. [glm](https://github.com/g-truc/glm) for OpenGL maths
