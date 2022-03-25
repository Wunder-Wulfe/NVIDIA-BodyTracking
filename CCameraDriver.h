#pragma once

struct CameraInfo
{
	int id, width, height;

	CameraInfo(cv::VideoCapture &cam, int c_id);
};

class CCameraDriver
{
	cv::VideoCapture currentCamera;
	CameraInfo* cinfo;
	int cameraIndex;
	void Cleanup();
	cv::Mat frame;
	bool working;
public:
	CCameraDriver(float scale = 1.0);
	~CCameraDriver() { Cleanup(); };
	void LoadCameras();
	void RunFrame();
	float resScale;
	bool show;
	std::vector<CameraInfo> cameras;

	void ChangeCamera(int up = 1);

	inline cv::Mat GetImage() { return frame; }
};

