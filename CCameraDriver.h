#pragma once
#include "CCallback.cpp"

struct CameraInfo
{
    int id, width, height;

    CameraInfo(cv::VideoCapture &cam, int c_id);
};

class CServerDriver;

class CCameraDriver
{
    cv::VideoCapture m_currentCamera;
    CameraInfo *m_cameraInfo;
    int m_cameraIndex;
    cv::Mat m_frame;
    std::vector<CameraInfo> m_cameras;
    bool m_working;

    void Cleanup();
protected:
    float m_resScale;

    friend class CServerDriver;
public:
    bool show;

    CCameraDriver(CServerDriver *driv, float scale = 1.0);
    ~CCameraDriver();

    void LoadCameras();
    void RunFrame();

    void ChangeCamera(int up = 1);

    inline const cv::Mat &GetImage() const { return m_frame; }
    inline const float GetFps() const { return (float)m_currentCamera.get(CV_CAP_PROP_FPS); }

    inline int GetWidth() const { return m_cameras[m_cameraIndex].width; }
    inline int GetHeight() const { return m_cameras[m_cameraIndex].height; }
    inline int GetScaledWidth() const { return (int)(GetWidth() * m_resScale); }
    inline int GetScaledHeight() const { return (int)(GetHeight() * m_resScale); }
    inline float GetScale() const { return m_resScale; }

    inline cv::VideoCaptureModes const GetMode() { return (cv::VideoCaptureModes)(int)m_currentCamera.get(CV_CAP_PROP_MODE); }

    CServerDriver *driver;

    CCallback<void(const CCameraDriver&, cv::Mat&)> imageChanged;
    CCallback<void(const CCameraDriver&, int)> cameraChanged;
};
