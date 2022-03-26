#pragma once

struct CameraInfo
{
    int id, width, height;

    CameraInfo(cv::VideoCapture &cam, int c_id);
};

class CCameraDriver
{
    cv::VideoCapture m_currentCamera;
    CameraInfo *m_cameraInfo;
    int m_cameraIndex;
    cv::Mat m_frame;
    std::vector<CameraInfo> m_cameras;
    bool m_working;
    float m_resScale;

    void Cleanup();
public:
    bool show;

    CCameraDriver(float scale = 1.0);
    ~CCameraDriver();

    void LoadCameras();
    void RunFrame();

    void ChangeCamera(int up = 1);

    inline const cv::Mat &GetImage() const { return m_frame; }
};
