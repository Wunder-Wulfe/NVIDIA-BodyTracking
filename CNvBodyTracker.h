#pragma once

enum class TRACKING_FLAG;
enum class BODY_JOINT;

class CNvBodyTracker
{
    bool m_imageLoaded;

    int m_inputImageWidth, m_inputImageHeight, m_inputImagePitch;

    NvAR_FeatureHandle m_bodyDetectHandle{}, m_keyPointDetectHandle{};
    CUstream m_stream{};
    NvCVImage m_inputImageBuffer{}, m_tmpImage{};
    std::vector<NvAR_Point2f> m_keypoints;
    std::vector<float> m_keypointsConfidence;
    std::vector<NvAR_Point3f> m_keypoints3D;
    std::vector<NvAR_Quaternion> m_jointAngles;
    unsigned int m_numKeyPoints;
    std::vector<NvAR_Point3f> m_referencePose;
    std::vector<NvAR_Rect> m_outputBBoxData;
    std::vector<float> m_outputBBoxConfData;
    NvAR_BBoxes m_outputBBoxes{};
    int m_batchSize;
    float m_confidence;

    TRACKING_FLAG m_flags;
    float m_fps;

    glm::mat4x4 m_camMatrix;

    std::vector<float> m_realConfidence;
    std::vector<glm::vec3> m_realKeypoints3D;
    std::vector<glm::quat> m_realJointAngles;

    void FillBatched(const std::vector<float> &from, std::vector<float> &to);
    void FillBatched(const std::vector<NvAR_Point3f> &from, std::vector<glm::vec3> &to);
    void FillBatched(const std::vector<NvAR_Quaternion> &from, std::vector<glm::quat> &to);

    void EmptyKeypoints();

    void ComputeAvgConfidence();

    template<class T>
    inline T &TableIndex(T *table, int index, int batch) { return table[batch * m_numKeyPoints + index]; }
    template<class T>
    inline const T &TableIndex(const std::vector<T> &vector, int index, int batch) { return vector[batch * m_numKeyPoints + index]; }

    inline glm::vec3 CastPoint(NvAR_Point3f point) { return glm::vec3(point.x, point.y, point.z); }
    inline glm::quat CastQuaternion(NvAR_Quaternion quat) { return glm::quat(quat.w, quat.x, quat.y, quat.z); }
public:
    bool useCudaGraph;
    float focalLength;
    bool stabilization;
    int nvARMode;
    int batchSize;
    bool trackingActive;
    float confidenceRequirement;

    void Initialize();
    void Initialize(int w, int h, int batch_size = 1);
    void ResizeImage(int w, int h);
    void Cleanup();

    void KeyInfoUpdated(bool override = false);

    inline float GetConfidence() const { return m_confidence; };
    inline float GetConfidence(BODY_JOINT role) const { return m_realConfidence[(int)role]; }

    inline void SetCamera(glm::vec3 pos, glm::quat rot) { m_camMatrix = glm::translate(glm::mat4_cast(rot), pos); }
    inline void RotateCamera(glm::quat rot) { m_camMatrix *= glm::mat4_cast(rot); }
    inline void MoveCamera(glm::vec3 pos) { m_camMatrix = glm::translate(m_camMatrix, pos); }
    inline glm::vec3 GetCameraPos() { return glm::vec3(m_camMatrix[3][0], m_camMatrix[3][1], m_camMatrix[3][2]); }
    inline glm::quat GetCameraRot() { return glm::quat_cast(m_camMatrix); }
    inline bool GetConfidenceAcceptable() { return m_confidence >= confidenceRequirement; }

    inline bool GetConfidenceAcceptable(BODY_JOINT role) const { return GetConfidence(role) >= confidenceRequirement; }

    inline int GetImageWidth() const { return m_inputImageWidth; }
    inline int GetImageHeight() const { return m_inputImageHeight; }

    inline void SetFPS(float f) { m_fps = f; }

    void RunFrame();

    CNvBodyTracker();
    ~CNvBodyTracker();
};

