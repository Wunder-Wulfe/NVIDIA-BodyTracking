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
	bool image_loaded;

	int input_image_width, input_image_height, input_image_pitch;

	NvAR_FeatureHandle bodyDetectHandle{}, keyPointDetectHandle{};
	CUstream stream{};
	NvCVImage inputImageBuffer{}, tmpImage{};
	std::vector<NvAR_Point2f> keypoints;
	std::vector<float> keypoints_confidence;
	std::vector<NvAR_Point3f> keypoints3D;
	std::vector<NvAR_Quaternion> jointAngles;
	unsigned int numKeyPoints;
	std::vector<NvAR_Point3f> referencePose;
	std::vector<NvAR_Rect> output_bbox_data;
	std::vector<float> output_bbox_conf_data;
	NvAR_BBoxes output_bboxes{};
	int _batchSize;

public:
	void Initialize();
	void Initialize(int w, int h, int batch_size=1);
	void ResizeImage(int w, int h);
	void Cleanup();
	void KeyInfoUpdated();

	bool useCudaGraph;
	float focalLength;
	bool stabilization;
	int nvARMode;
	int batchSize;
	bool trackingActive;

	glm::mat4x4 camMatrix;
	inline void SetCamera(glm::vec3 pos, glm::quat rot) { camMatrix = glm::mat4_cast(rot); camMatrix += pos; }
	inline void RotateCamera(glm::quat rot) { camMatrix *= rot; }
	inline void MoveCamera(glm::vec3 pos) { camMatrix += pos; }
	inline glm::vec3 GetCameraPos() { return glm::vec3(camMatrix[3][0], camMatrix[3][1], camMatrix[3][2]); }
	inline glm::quat GetCameraRot() { return glm::quat_cast(camMatrix); }

	inline int ImageWidth() { return input_image_width; }
	inline int ImageHeight() { return input_image_height; }

	CBodyTrackDriver();
	~CBodyTrackDriver();
};

