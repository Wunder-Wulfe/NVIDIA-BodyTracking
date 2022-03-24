#pragma once

enum class BODY_JOINTS {
	PELVIS,
	LEFT_HIP,
	RIGHT_HIP,
	TORSO,
	LEFT_KNEE,
	RIGHT_KNEE,
	NECK,
	LEFT_ANKLE,
	RIGHT_ANKLE,
	LEFT_BIG_TOE,
	RIGHT_BIG_TOE,
	LEFT_SMALL_TOE,
	RIGHT_SMALL_TOE,
	LEFT_HEEL,
	RIGHT_HEEL,
	NOSE,
	LEFT_EYE,
	RIGHT_EYE,
	LEFT_EAR,
	RIGHT_EAR,
	LEFT_SHOULDER,
	RIGHT_SHOULDER,
	LEFT_ELBOW,
	RIGHT_ELBOW,
	LEFT_WRIST,
	RIGHT_WRIST,
	LEFT_PINKY_KNUCKLE,
	RIGHT_PINKY_KNUCKLE,
	LEFT_MIDDLE_TIP,
	RIGHT_MIDDLE_TIP,
	LEFT_INDEX_KNUCKLE,
	RIGHT_INDEX_KNUCKLE,
	LEFT_THUMB_TIP,
	RIGHT_THUMB_TIP
};

class CBodyTrackDriver
{
	bool trackingActive;
	bool stabilization;
	bool image_loaded;

	int input_image_width, input_image_height;

	NvAR_FeatureHandle bodyDetectHandle{};
	CUstream stream{};
	NvCVImage inputImageBuffer{}, tmpImage{};
	std::vector<NvAR_Point2f> keypoints;
	std::vector<float> keypoints_confidence;
	std::vector<NvAR_Point3f> keypoints3D;
	std::vector<NvAR_Quaternion> jointAngles;
	unsigned int numKeyPoints;
	std::vector<NvAR_Point3f> referencePose;
	NvAR_FeatureHandle keyPointDetectHandle{};
	bool useCudaGraph;
	int batchSize;

public:
	void Initialize();
	void ResizeImage(int w, int h);
	void Cleanup();
	void SetBatchSize(int b) { batchSize = b; }
	void SetUseCudaGraph(bool b) { useCudaGraph = b; }
	CBodyTrackDriver();
	~CBodyTrackDriver();
};

