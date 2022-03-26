#include "pch.h"
#include "CNvBodyTracker.h"
#include "CCommon.h"

char *g_nvARSDKPath = nullptr;

CNvBodyTracker::CNvBodyTracker()
{
    trackingActive = false;
    stabilization = true;
    m_imageLoaded = false;
    useCudaGraph = true;
    nvARMode = 1;
    focalLength = 800.0f;
    batchSize = 1;
    m_batchSize = -1;
    m_keyPointDetectHandle = nullptr;
    m_bodyDetectHandle = nullptr;
}

void CNvBodyTracker::KeyInfoUpdated(bool override)
{
    int _nkp = m_numKeyPoints;

    if(batchSize != m_batchSize || override)
    {
        if(m_keyPointDetectHandle != nullptr)
        {
            NvAR_Destroy(m_keyPointDetectHandle);
            m_keyPointDetectHandle = nullptr;
        }
        NvAR_Create(NvAR_Feature_BodyPoseEstimation, &m_keyPointDetectHandle);
        NvAR_SetString(m_keyPointDetectHandle, NvAR_Parameter_Config(ModelDir), "");
        NvAR_SetCudaStream(m_keyPointDetectHandle, NvAR_Parameter_Config(CUDAStream), m_stream);
        NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(BatchSize), batchSize);
    }
    NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(Mode), nvARMode);
    NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(Temporal), stabilization);
    NvAR_SetF32(m_keyPointDetectHandle, NvAR_Parameter_Config(FocalLength), focalLength);
    NvAR_SetF32(m_keyPointDetectHandle, NvAR_Parameter_Config(UseCudaGraph), useCudaGraph);
    NvAR_Load(m_keyPointDetectHandle);

    NvAR_GetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(NumKeyPoints), &m_numKeyPoints);
    if(batchSize != m_batchSize || m_numKeyPoints != _nkp || override)
    {
        m_keypoints.assign(batchSize * m_numKeyPoints, { 0.f, 0.f });
        m_keypoints3D.assign(batchSize * m_numKeyPoints, { 0.f, 0.f, 0.f });
        m_jointAngles.assign(batchSize * m_numKeyPoints, { 0.f, 0.f, 0.f, 1.f });
        m_keypointsConfidence.assign(batchSize * m_numKeyPoints, 0.f);
        m_referencePose.assign(m_numKeyPoints, { 0.f, 0.f, 0.f });

        EmptyKeypoints();

        const void *pReferencePose;
        NvAR_GetObject(m_keyPointDetectHandle, NvAR_Parameter_Config(ReferencePose), &pReferencePose,
            sizeof(NvAR_Point3f));
        memcpy(m_referencePose.data(), pReferencePose, sizeof(NvAR_Point3f) * m_numKeyPoints);

        NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Output(KeyPoints), m_keypoints.data(),
            sizeof(NvAR_Point2f));
        NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Output(KeyPoints3D), m_keypoints3D.data(),
            sizeof(NvAR_Point3f));
        NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Output(JointAngles), m_jointAngles.data(),
            sizeof(NvAR_Quaternion));
        NvAR_SetF32Array(m_keyPointDetectHandle, NvAR_Parameter_Output(KeyPointsConfidence),
            m_keypointsConfidence.data(), batchSize * m_numKeyPoints);
    }

    m_batchSize = batchSize;
}

void CNvBodyTracker::Initialize()
{
    if(m_stream != nullptr)
        Cleanup();

    unsigned int output_bbox_size;
    NvAR_CudaStreamCreate(&m_stream);
    if(m_bodyDetectHandle == nullptr)
    {
        NvAR_Create(NvAR_Feature_BodyDetection, &m_bodyDetectHandle);
        NvAR_SetString(m_bodyDetectHandle, NvAR_Parameter_Config(ModelDir), "");
        NvAR_SetCudaStream(m_bodyDetectHandle, NvAR_Parameter_Config(CUDAStream), m_stream);
        NvAR_SetU32(m_bodyDetectHandle, NvAR_Parameter_Config(Temporal), stabilization);
        NvAR_Load(m_bodyDetectHandle);
    }

    KeyInfoUpdated(true);

    output_bbox_size = batchSize;
    if(!stabilization) output_bbox_size = 25;
    m_outputBBoxData.assign(output_bbox_size, { 0.f, 0.f, 0.f, 0.f });
    m_outputBBoxes.boxes = m_outputBBoxData.data();
    m_outputBBoxes.max_boxes = (uint8_t)output_bbox_size;
    m_outputBBoxes.num_boxes = (uint8_t)output_bbox_size;
    NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Output(BoundingBoxes), &m_outputBBoxes, sizeof(NvAR_BBoxes));
}

void CNvBodyTracker::Initialize(int w, int h, int batch_size)
{
    batchSize = batch_size;
    ResizeImage(w, h);
    Initialize();
}

void CNvBodyTracker::ResizeImage(int w, int h)
{
    m_inputImageWidth = w;
    m_inputImageHeight = h;
    m_inputImagePitch = 3 * m_inputImageWidth * sizeof(unsigned char);

    if(m_imageLoaded)
        NvCVImage_Dealloc(&m_inputImageBuffer);
    NvCVImage_Alloc(&m_inputImageBuffer, m_inputImageWidth, m_inputImageHeight, NVCV_BGR, NVCV_U8,
        NVCV_CHUNKY, NVCV_GPU, 1);
    NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Input(Image), &m_inputImageBuffer, sizeof(NvCVImage));
    m_imageLoaded = true;
}

CNvBodyTracker::~CNvBodyTracker()
{
    Cleanup();
}

void CNvBodyTracker::Cleanup()
{
    if(m_keyPointDetectHandle != nullptr)
    {
        NvAR_Destroy(m_keyPointDetectHandle);
        m_keyPointDetectHandle = nullptr;
    }
    if(m_bodyDetectHandle != nullptr)
    {
        NvAR_Destroy(m_bodyDetectHandle);
        m_bodyDetectHandle = nullptr;
    }
    if(m_stream != nullptr)
    {
        NvAR_CudaStreamDestroy(m_stream);
        m_stream = nullptr;
    }
    if(m_imageLoaded)
        NvCVImage_Dealloc(&m_inputImageBuffer);
}

void CNvBodyTracker::FillBatched(const std::vector<NvAR_Quaternion> &from, std::vector<glm::quat> &to)
{
    int index, batch;
    for(index = 0; index < (int)m_numKeyPoints; index++)
    {
        to[index] = CastQuaternion(from[index]) / (float)batchSize;
    }

    for(batch = 1; batch < batchSize; batch++)
    {
        for(index = 0; index < (int)m_numKeyPoints; index++)
        {
            to[index] += CastQuaternion(TableIndex(from, index, batch)) / (float)batchSize;
        }
    }
}

void CNvBodyTracker::FillBatched(const std::vector<NvAR_Point3f> &from, std::vector<glm::vec3> &to)
{
    int index, batch;
    for(index = 0; index < (int)m_numKeyPoints; index++)
    {
        to[index] = CastPoint(from[index]) / (float)batchSize;
    }

    for(batch = 1; batch < batchSize; batch++)
    {
        for(index = 0; index < (int)m_numKeyPoints; index++)
        {
            to[index] += CastPoint(TableIndex(from, index, batch)) / (float)batchSize;
        }
    }
}

void CNvBodyTracker::ComputeAvgConfidence()
{
    float avg = 0.0;
    int batch, index;
    for(batch = 0; batch < batchSize; batch++)
    {
        for(index = 0; index < (int)m_numKeyPoints; index++)
        {
            avg += TableIndex(m_keypointsConfidence, index, batch);
        }
    }
    m_confidence = avg / (batchSize + m_numKeyPoints);
}

void CNvBodyTracker::EmptyKeypoints()
{
    m_realKeypoints3D.assign(m_numKeyPoints, { 0.f, 0.f, 0.f });
    m_realJointAngles.assign(m_numKeyPoints, { 0.f, 0.f, 0.f, 0.f });
}

void CNvBodyTracker::RunFrame()
{
    if(trackingActive)
    {
        NvAR_Run(m_keyPointDetectHandle);
        ComputeAvgConfidence();
        if(m_confidence >= confidenceRequirement)
        {
            FillBatched(m_keypoints3D, m_realKeypoints3D);
            FillBatched(m_jointAngles, m_realJointAngles);
        }
        else
            EmptyKeypoints();
    }
    else
        EmptyKeypoints();
}