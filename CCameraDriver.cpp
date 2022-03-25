#include "pch.h"
#include "CCameraDriver.h"
#include "CCommon.h"

CameraInfo::CameraInfo(cv::VideoCapture& cam, int c_id)
{
    width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
    height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);
    id = c_id;
}

void CCameraDriver::LoadCameras()
{
    currentCamera.release();
	cameras.clear();

    cv::VideoCapture camera;
    int device_counts = 0;
    while (true) {
        if (!camera.open(device_counts)) {
            break;
        }
        cameras.push_back(CameraInfo(camera, device_counts));
        sprintf_s(logging_buffer, LOG_BUFFER_SIZE, "Found device at index %d (%dx%d)\n", device_counts, cameras[device_counts].width, cameras[device_counts].height);
        vr::VRDriverLog()->Log(logging_buffer);
        device_counts++;
    }

    cameraIndex = 0;
    ChangeCamera(0);
}

CCameraDriver::CCameraDriver(float scale) : currentCamera() {
    resScale = scale; 
    cameraIndex = 0;
    show = false;
    working = false;
    cinfo = nullptr;
}

void CCameraDriver::RunFrame()
{
    currentCamera >> frame;
    if (show && !frame.empty())
        cv::imshow("Input", frame);
}

void CCameraDriver::ChangeCamera(int up)
{
    if (cameras.size() > 0)
    {
        cameraIndex = (cameraIndex + up) % cameras.size();
        cinfo = &cameras[cameraIndex];
        currentCamera.open(cinfo->id);
        currentCamera.set(cv::CAP_PROP_FRAME_WIDTH, (int)(cinfo->width * resScale));
        currentCamera.set(cv::CAP_PROP_FRAME_HEIGHT, (int)(cinfo->height * resScale));

        sprintf_s(logging_buffer, LOG_BUFFER_SIZE, "Switching to camera of index %d\n", cinfo->id);
        vr::VRDriverLog()->Log(logging_buffer);
    }
    else
    {
        cameraIndex = 0;
        currentCamera.release();
    }
}

void CCameraDriver::Cleanup()
{
    cv::destroyAllWindows();
    working = false;
    currentCamera.release();
}