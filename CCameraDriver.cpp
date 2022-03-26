#include "pch.h"
#include "CCameraDriver.h"
#include "CCommon.h"

CameraInfo::CameraInfo(cv::VideoCapture &cam, int c_id)
{
    width = (int)cam.get(cv::CAP_PROP_FRAME_WIDTH);
    height = (int)cam.get(cv::CAP_PROP_FRAME_HEIGHT);
    id = c_id;
}

void CCameraDriver::LoadCameras()
{
    m_currentCamera.release();
    m_cameras.clear();

    cv::VideoCapture camera;
    int device_counts = 0;
    while(true)
    {
        if(!camera.open(device_counts)) break;

        m_cameras.push_back(CameraInfo(camera, device_counts));

        vr_log("Found device at index %d (%dx%d)\n", device_counts, m_cameras[device_counts].width, m_cameras[device_counts].height);

        device_counts++;
    }

    m_cameraIndex = 0;
    ChangeCamera(0);
}

CCameraDriver::CCameraDriver(float scale) : m_currentCamera()
{
    m_resScale = scale;
    m_cameraIndex = 0;
    show = false;
    m_working = false;
    m_cameraInfo = nullptr;
}
CCameraDriver::~CCameraDriver()
{
    Cleanup();
}

void CCameraDriver::RunFrame()
{
    m_currentCamera >> m_frame;
    /*if (show && !m_frame.empty())
        cv::imshow("Input", m_frame);*/
}

void CCameraDriver::ChangeCamera(int up)
{
    if(m_cameras.size() > 0)
    {
        m_cameraIndex = (m_cameraIndex + up) % m_cameras.size();
        m_cameraInfo = &m_cameras[m_cameraIndex];
        m_currentCamera.open(m_cameraInfo->id);
        m_currentCamera.set(cv::CAP_PROP_FRAME_WIDTH, (int)(m_cameraInfo->width * m_resScale));
        m_currentCamera.set(cv::CAP_PROP_FRAME_HEIGHT, (int)(m_cameraInfo->height * m_resScale));

        vr_log("Switching to camera of index %d (%dx%d)\n", m_cameraInfo->id, m_cameraInfo->width, m_cameraInfo->height);
    }
    else
    {
        m_cameraIndex = 0;
        m_currentCamera.release();
    }
}

void CCameraDriver::Cleanup()
{
    cv::destroyAllWindows();
    m_working = false;
    m_currentCamera.release();
}