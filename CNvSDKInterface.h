#pragma once

enum class TRACKING_FLAG;
enum class BODY_JOINT;
class CServerDriver;

//  NVIDIA AR SDK Interface, designed to simplify and handle the interpretation of data from the SDK
class CNvSDKInterface
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

    void RotateBatched(std::vector<float> &to);
    void RotateBatched(std::vector<NvAR_Point3f> &to);
    void RotateBatched(std::vector<NvAR_Quaternion> &to);

    void EmptyKeypoints();

    void ComputeAvgConfidence();

    template<class T>
    inline T TableIndex(T *table, int index, int batch) { return table[batch * m_numKeyPoints + index]; }
    template<class T>
    inline const T TableIndex(const std::vector<T> &vector, int index, int batch) { return vector[batch * m_numKeyPoints + index]; }

protected:
    inline const std::vector<float> GetRealConfidence() const { return m_realConfidence; }
    inline const std::vector<glm::vec3> GetRealKeypoints() const { return m_realKeypoints3D; }
    inline const std::vector<glm::quat> GetRealAngles() const { return m_realJointAngles; }

    static inline const glm::quat SumQuat(const glm::quat &q1, const glm::quat &q2) {
        return glm::quat(q1.w + q2.w, q1.x + q2.x, q1.y + q2.y, q1.z + q2.z);
    }

    static inline const glm::quat AvgQuat(const glm::quat &q1, const uint count) {
        return glm::quat(q1.w / count, q1.x / count, q1.y / count, q1.z / count);
    }

    glm::vec3 m_axisScale;
    glm::vec3 m_offset;
    bool m_alignHMD;

    static inline const glm::vec3 CastPoint(const NvAR_Point3f &point) { return glm::vec3(point.x, point.y, point.z); }
    static inline const glm::quat CastQuaternion(const NvAR_Quaternion &quat) { return glm::quat(quat.w, quat.x, quat.y, quat.z); }
    static inline const glm::mat4x4 CastMatrix(const glm::vec3 &point, const glm::quat &quat) { return Slide(glm::mat4_cast(quat), point); }
    static inline const glm::mat4x4 CastMatrix(const NvAR_Point3f &point, const NvAR_Quaternion &quat) { return CastMatrix(CastPoint(point), CastQuaternion(quat)); }

    void AlignToHMD(const vr::TrackedDevicePose_t &pose);

    friend class CServerDriver;
public:
    CServerDriver *driver;

    static inline const glm::mat4x4 Slide(glm::mat4x4 mat, const glm::vec3 vector) {
        mat[3][0] += vector.x;
        mat[3][1] += vector.y;
        mat[3][2] += vector.z;
        return mat;
    }

    static inline const glm::mat4x4 InverseRotation(glm::mat4x4 mat) {
        glm::vec3 temp = glm::vec3(mat[3]);
        mat = glm::inverse(mat);
        mat[3][0] = temp.x;
        mat[3][1] = temp.y;
        mat[3][2] = temp.z;
        return mat;
    }

    static inline const float ExtractAngleX(const glm::mat4x4 &mat)
    {
        float t_x, t_y, t_z;
        glm::extractEulerAngleYXZ(mat, t_x, t_y, t_z);
        return t_x;
    }

    static inline const float ExtractAngleY(const glm::mat4x4 &mat)
    {
        float t_x, t_y, t_z;
        glm::extractEulerAngleYXZ(mat, t_x, t_y, t_z);
        return t_y;
    }
    static inline const float ExtractAngleZ(const glm::mat4x4 &mat)
    {
        float t_x, t_y, t_z;
        glm::extractEulerAngleYXZ(mat, t_x, t_y, t_z);
        return t_z;
    }

    bool useCudaGraph;
    float focalLength;
    bool stabilization;
    int nvARMode;
    int batchSize;
    int realBatches;
    bool trackingActive;
    float confidenceRequirement;

    bool ready;

    void Initialize();
    void Initialize(int w, int h, int batch_size = 1);
    void ResizeImage(int w, int h);
    void Cleanup();

    void KeyInfoUpdated(bool override = false);

    inline float GetConfidence() const { return m_confidence; };
    inline float GetConfidence(BODY_JOINT role) const { return m_realConfidence[(int)role]; }

    inline void SetCamera(glm::vec3 pos, glm::quat rot) { m_camMatrix = Slide(glm::mat4_cast(rot), pos); }
    inline void RotateCamera(glm::quat rot) { m_camMatrix *= glm::mat4_cast(rot); }
    inline void MoveCamera(glm::vec3 pos) { m_camMatrix = Slide(m_camMatrix, pos); }
    inline void SetCameraRotation(glm::quat rot) { SetCamera(GetCameraPos(), rot); }
    inline void TranslateCamera(glm::vec3 pos) { m_camMatrix = glm::translate(m_camMatrix, pos); }
    inline const glm::vec3 GetCameraPos() const { return glm::vec3(m_camMatrix[3][0], m_camMatrix[3][1], m_camMatrix[3][2]); }
    inline const glm::quat GetCameraRot() const { return glm::quat_cast(m_camMatrix); }
    inline const glm::mat4x4 GetCameraMatrix() const { return m_camMatrix; }
    inline bool GetConfidenceAcceptable() const { return m_confidence >= confidenceRequirement; }

    static inline const glm::quat MirrorQuaternionX(const glm::quat &ref)
    {
        return glm::quat(ref.w, ref.x, -ref.y, -ref.z);
    }
    static inline const glm::quat MirrorQuaternionY(const glm::quat &ref)
    {
        return glm::quat(ref.w, -ref.x, ref.y, -ref.z);
    }
    static inline const glm::quat MirrorQuaternionZ(const glm::quat &ref)
    {
        return glm::quat(ref.w, -ref.x, -ref.y, ref.z);
    }

    static inline const glm::mat4x4 MirrorRotationX(const glm::mat4x4 &ref)
    {
        return CastMatrix(glm::vec3(ref[3]), MirrorQuaternionX(glm::quat_cast(ref)));
    }
    static inline const glm::mat4x4 MirrorRotationY(const glm::mat4x4 &ref)
    {
        return CastMatrix(glm::vec3(ref[3]), MirrorQuaternionY(glm::quat_cast(ref)));
    }
    static inline const glm::mat4x4 MirrorRotationZ(const glm::mat4x4 &ref)
    {
        return CastMatrix(glm::vec3(ref[3]), MirrorQuaternionZ(glm::quat_cast(ref)));
    }

    void DebugSequence(const std::vector<float> conf) const;
    void DebugSequence(const std::vector<NvAR_Point3f> kep) const;
    void DebugSequence(const std::vector<NvAR_Quaternion> kep) const;
    void DebugSequence(const std::vector<glm::vec3> kep) const;
    void DebugSequence(const std::vector<glm::quat> rot) const;

    void LoadImageFromCam(const cv::VideoCapture &cam);
    void UpdateImageFromCam(const cv::Mat image);

    inline bool GetConfidenceAcceptable(BODY_JOINT role) const { return GetConfidence(role) >= confidenceRequirement; }
    inline bool GetConfidenceAcceptable(BODY_JOINT role, BODY_JOINT secondary) const { return (GetConfidence(role) + GetConfidence(secondary)) / 2.f >= confidenceRequirement; }

    inline const glm::mat4x4 GetTransform(BODY_JOINT role) const { return CastMatrix(GetPosition(role), GetRotation(role)); }
    inline const glm::mat4x4 GetTransform(BODY_JOINT role, BODY_JOINT rotation_owner) const { return CastMatrix(GetPosition(role), GetRotation(rotation_owner)); }
    inline const glm::vec3 GetPosition(BODY_JOINT role) const { return m_realKeypoints3D[(int)role]; }
    inline const glm::vec3 GetPosition(BODY_JOINT role, BODY_JOINT secondary) const { return glm::mix(GetPosition(role), GetPosition(secondary), .5f); }
    inline const glm::quat GetRotation(BODY_JOINT role) const { return m_realJointAngles[(int)role]; }
    inline const glm::quat GetRotation(BODY_JOINT role, BODY_JOINT secondary) const { return glm::slerp(GetRotation(role), GetRotation(secondary), .5f); }
    inline const glm::mat4x4 GetAverageTransform(BODY_JOINT from, BODY_JOINT to) const { return InterpolateMatrix(GetTransform(from), GetTransform(to), .5f); }
    inline const glm::mat4x4 GetInterpolatedTransform(BODY_JOINT from, BODY_JOINT to, BODY_JOINT rotation_owner, float alpha = 0.f) const
    { 
        return CastMatrix(glm::mix(GetPosition(from), GetPosition(to), alpha), GetRotation(rotation_owner));
    }
    inline const glm::mat4x4 GetInterpolatedTransformMulti(BODY_JOINT from, BODY_JOINT middle, BODY_JOINT to, float alpha = 0.f) const
    {
        if (alpha > 0.f)
            return GetInterpolatedTransform(middle, to, to, alpha);
        else
            return GetInterpolatedTransform(middle, from, middle, -alpha);
    }

    static inline const glm::vec3 ObjectToWorldVector(const glm::mat4x4 &mat, const glm::vec3 &vec)
    {
        return glm::translate(mat, vec)[3];
    }
    static inline const glm::vec3 WorldToObjectVector(const glm::mat4x4 &mat, const glm::vec3 &vec)
    {
        return glm::translate(glm::inverse(mat), vec)[3];
    }
    inline const glm::vec3 CamToWorld(const glm::vec3 &input) const {
        return ObjectToWorldVector(GetCameraMatrix(), input);
    }

    inline const glm::vec3 WorldToCam(const glm::vec3 &input) const {
        return WorldToObjectVector(GetCameraMatrix(), input);
    }

    static inline const glm::mat4x4 InterpolateMatrix(const glm::mat4x4 &mat1, const glm::mat4x4 &mat2, const float &delta)
    {
        glm::mat4x4 output = glm::mat4_cast(glm::slerp(glm::quat_cast(mat1), glm::quat_cast(mat2), delta));
        output[3] = mat2[3] * delta + mat1[3] * (1.f - delta);
        return output;
    }

    inline int GetImageWidth() const { return m_inputImageWidth; }
    inline int GetImageHeight() const { return m_inputImageHeight; }

    inline void SetFPS(float f) { m_fps = f; }

    void RunFrame();

    CNvSDKInterface();
    ~CNvSDKInterface();
};

