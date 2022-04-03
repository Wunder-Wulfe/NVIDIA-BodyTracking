# NVIDIA BodyTracking SteamVR Driver

![Powered By: NVIDIA RTX](https://user-images.githubusercontent.com/29297318/159997280-2131e876-42bd-4c8b-9472-7c88c6c7ba60.png)

----

## Requirements

* Windows OS
* NVIDIA RTX GPU, 20-series or 30-series
* Any camera that can be linked to your computer for realtime capture, *i.e.* a webcam, mobile device, or go-pro
* Installation of the [NVIDIA Broadcast AR SDK](https://www.nvidia.com/en-us/geforce/broadcasting/broadcast-sdk/resources/) ([20-series](https://international.download.nvidia.com/Windows/broadcast/sdk/v0.7.6/nvidia_ar_sdk_installer_turing.exe) | [30-series](https://international.download.nvidia.com/Windows/broadcast/sdk/0.7.6/nvidia_ar_sdk_installer_ampere.exe))

---

This is a **WIP** driver designed to use the NVIDIA AR SDK for camera-based body tracking.
For now, most of the code is based on the [AR SDK examples](https://github.com/NVIDIA/MAXINE-AR-SDK), [Mediapipe-VR-Fullbody-Tracking](https://github.com/ju1ce/Mediapipe-VR-Fullbody-Tracking), and [driver_kinectV2](https://github.com/SDraw/driver_kinectV2/blob/master/driver_kinectV2).

https://user-images.githubusercontent.com/29297318/160537122-21e5f4ce-0a91-44e1-828c-0a6ca8b94717.mp4

[Discord Server](https://discord.gg/XjkyuwRW6Z)


Current keybinds can be found in the vrserver.txt log, but here are the current bindings
![image](https://user-images.githubusercontent.com/29297318/160949112-6ada034d-ef79-4208-ae32-474c637a7785.png)

This is a **SteamVR Driver**, and as such will need to grab the [latest release](https://github.com/Wunder-Wulfe/NVIDIA-BodyTracking/releases) and install it within a directory under your SteamVR application with this file path: 

``Steam\steamapps\common\SteamVR\drivers\``


The settings file for configuring the driver can be located under 

``Steam\steamapps\common\SteamVR\drivers\nvidiaBodyTracking\bin\win64\settings.ini``

If your file path contains ``..\nvidiaBodyTracking\nvidiaBodyTracking\..``, reduce it to one folder only.

----

## FAQ

1. **Can I use this with an AMD GPU, or an RTX 1660?**
    * No. This SDK only works with 20-series and 30-series NVIDIA RTX cards.

2. **I only have a Kinect camera / crappy laptop camera. Will this still work?**
    * This solution works on any and all cameras, provided they can record in realtime. The quality of the tracking will reflect the quality of the camera. Generally a higher refresh rate will result in smoother tracking.
3. **Can I use my iPhone or Android device as a camera?**
    * Yes. As long as you have softwares to allow the phone to be used as a camera on your computer, you can use the phone.
4. **The driver did not pick the camera that I wanted to use. Is my camera not working?**
    * If you can open the windows Camera app and switch to your camera, it is functional. The driver will be able to see your camera as long as it has been turned on before SteamVR has been started. Check the keybinds above to learn how to change your camera.
5. **Can I combine this form of tracking with SlimeVR, Vive, etc.?**
    * Yes. Edit your ``settings.ini`` file and choose which trackers you would like to enable. If you already have vive trackers for your feet and hips for example, you should turn off tracking for both of those joints.
6. **Can this track me as I walk around my room and turn around?**
    * Yes. It should be able to track your body sufficiently as you navigate your playspace, as long as the camera can see your entire body.
7. **Can this track me while I am sitting or lying down?**
    * To some extent, yes. If there is a lot of occlusion going on with the body, and the perspective of the camera makes it too difficult to figure out where things are, the legs may lose tracking.
8. **All of a sudden, my trackers stopped working ingame. What happened?**
    * Make sure that the camera is still set to the one you would like to be using, and if you would like to prevent the trackers from turning off when the system isn't confident of their location, edit your ``settings.ini`` file and set the ``ConfidenceMin`` to a lower value, like `0.0` or `0.01`.
9. **I already use a tracking system. Should I still try this out?**
    * Probably. As long as you meet the requirements, that is all you need in order to start tracking yourself, along with the calibration of your camera in SteamVR. You can disable this driver in your startup settings for SteamVR, or delete the driver folder to uninstall it. Even if it is just out of curiosity, it should not require much setup.
10. **What is "HMD Alignment?"**
    * This is a system I am using to attempt to correct for drifting issues with the tracking of your body, and to reduce the need to worry about the position of the camera. It will be used in tandem with other features in the future to calibrate your body automatically.
11. **Can this be used in Desktop mode?**
    * Technically yes. If you use other drivers to be able to play SteamVR "headless" mode, you should be able to play on desktop, although your head will not be tracked (currently).
12. **Can I use this in NeosVR and ChilloutVR, or only in VRChat?**
    * This is a SteamVR driver, not a game mod. It can be used in any VR title that offers support for body trackers.
13. **I like this project! How can I contribute?**
    * If you would like to contribute to the project, start by creating a fork and working on some changes to it. When you have a contribution to make, create a pull request to the main branch. I will take a look at it and merge if appropriate.
14. **Is this a virus?**
    * No. You can inspect the source code for yourself, and it has been verified by other developers as well even before I began distributing it. It should not bring up any flags on [VirusTotal](https://www.virustotal.com/gui/file/7027b3a6f529d57dad192fc274225daea5120793310425abc23ec2e847545a8a?nocache=1), and all of the executables that are released are compiled directly from the current state of the source code on GitHub.
15. **What are your recommended settings for the .ini file?**
    * My recommended settings will always be the [default settings file](https://github.com/Wunder-Wulfe/NVIDIA-BodyTracking/blob/main/settings.ini) that I update as I make changes to the program. All of the settings are well documented so it should be no mystery what a majority of them do.
16. **Where can I report bugs or issues with the program?**
    * Join the [Discord Server](https://discord.gg/XjkyuwRW6Z) and create a thread within the [#issues](https://discord.com/channels/956633023011520593/956910830627213402) channel, or use the [Issues](https://github.com/Wunder-Wulfe/NVIDIA-BodyTracking/issues) tab on this project to do it on GitHub. I would prefer reports to go through Discord where I can more readily / easily see it.

----

In order to use any of the executables, you are required to download and install the [NVIDIA Broadcast AR SDK](https://www.nvidia.com/en-us/geforce/broadcasting/broadcast-sdk/resources/).
*(Note: As of right now, it works only on RTX 20 and 30 series cards. Other cards are untested)*

NVIDIA Library used: [MAXINE AR SDK](https://github.com/NVIDIA/MAXINE-AR-SDK)

Additional libraries used:

1. [OpenCV](https://github.com/opencv/opencv) for image/video capture
2. [SimpleIni](https://github.com/brofield/simpleini) for the configuration file
3. [glm](https://github.com/g-truc/glm) for OpenGL maths
