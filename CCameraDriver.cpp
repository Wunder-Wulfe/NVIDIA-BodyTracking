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
    if (m_cameras.size() > 0)
    {
        m_currentCamera.release();
        m_cameras.clear();
    }
    cv::VideoCapture camera;
    int device_counts = 0;
    while(true)
    {
        if(!camera.open(device_counts)) break;

        m_cameras.push_back(CameraInfo(camera, device_counts));

        vr_log("Found camera at index %d (%dx%d)\n", device_counts, m_cameras[device_counts].width, m_cameras[device_counts].height);

        device_counts++;
    }

    m_cameraIndex = 0;
    ChangeCamera(0);
}

CCameraDriver::CCameraDriver(CServerDriver *driv, float scale) : m_currentCamera(), imageChanged(), cameraChanged(), m_camMutex{}
{
    m_resScale = scale;
    m_cameraIndex = 0;
    show = false;
    m_working = true;
    m_cameraInfo = nullptr;
    driver = driv;
}

CCameraDriver::~CCameraDriver()
{
    Cleanup();
}

void CCameraDriver::RunAsync()
{
    m_working = true;
    vr_log("Initializing main camera loop");
    while (m_working)
    {
        m_camMutex.lock();
        if (m_currentCamera.read(m_frame) && !m_frame.empty())
        {
            m_camMutex.unlock();
            if (show) cv::imshow("Input", m_frame);
            imageChanged(*this, m_frame);
        }
        else
            m_camMutex.unlock();
        cv::waitKey(1);
    }
}

void CCameraDriver::ChangeCamera(int up)
{
    if(m_cameras.size() > 0)
    {
        m_cameraIndex = (m_cameraIndex + up) % m_cameras.size();
        m_cameraInfo = &m_cameras[m_cameraIndex];
        m_currentCamera.open(m_cameraInfo->id);
        m_currentCamera.set(cv::CAP_PROP_FRAME_WIDTH, GetScaledWidth());
        m_currentCamera.set(cv::CAP_PROP_FRAME_HEIGHT, GetScaledHeight());
        
        vr_log("Switching to camera of index %d (%dx%d) (%.2f fps)\n", m_cameraInfo->id, GetWidth(), GetHeight(), GetFps());
        cameraChanged(*this, m_cameraIndex);
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
    m_cameras.clear();
}