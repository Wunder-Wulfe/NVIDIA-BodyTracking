#pragma once

enum class TRACKING_FLAG;

struct Proportions
{
	float
		elbowOffset,
		kneeOffset,
		hipOffset,
		chestOffset;
};

class CNvBodyTracker
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
	float confidence;

	Proportions proportions;
	TRACKING_FLAG flags;

	std::vector<glm::vec3> real_keypoints3D;
	std::vector<glm::quat> real_jointAngles;

	void FillBatched(std::vector<NvAR_Point3f> &from, std::vector<glm::vec3> &to);
	void FillBatched(std::vector<NvAR_Quaternion>& from, std::vector<glm::quat>& to);

	void EmptyKeypoints();

	void ComputeAvgConfidence();
			
	template<class T>
	inline T& TableIndex(T* table, int index, int batch) { return table[batch * numKeyPoints + index]; }
	template<class T>
	inline T& TableIndex(std::vector<T> &vector, int index, int batch) { return vector[batch * numKeyPoints + index]; }

	inline glm::vec3 CastPoint(NvAR_Point3f point) { return glm::vec3(point.x, point.y, point.z); }
	inline glm::quat CastQuaternion(NvAR_Quaternion quat) { return glm::quat(quat.w, quat.x, quat.y, quat.z); }

public:
	void Initialize();
	void Initialize(int w, int h, int batch_size=1);
	void ResizeImage(int w, int h);
	void Cleanup();

	void KeyInfoUpdated(bool override = false);

	bool useCudaGraph;
	float focalLength;
	bool stabilization;
	int nvARMode;
	int batchSize;
	bool trackingActive;
	float confidenceRequirement;

	inline float GetConfidence() const { return confidence; };

	glm::mat4x4 camMatrix;
	inline void SetCamera(glm::vec3 pos, glm::quat rot) { camMatrix = glm::translate(glm::mat4_cast(rot), pos); }
	inline void RotateCamera(glm::quat rot) { camMatrix *= glm::mat4_cast(rot); }
	inline void MoveCamera(glm::vec3 pos) { camMatrix = glm::translate(camMatrix, pos); }
	inline glm::vec3 GetCameraPos() { return glm::vec3(camMatrix[3][0], camMatrix[3][1], camMatrix[3][2]); }
	inline glm::quat GetCameraRot() { return glm::quat_cast(camMatrix); }
	inline bool GetConfidenceAcceptable() { return confidence >= confidenceRequirement; }

	inline int GetImageWidth() const { return input_image_width; }
	inline int GetImageHeight() const { return input_image_height; }

	void RunFrame();

	CNvBodyTracker();
	~CNvBodyTracker();
};

