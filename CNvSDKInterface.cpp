#include "pch.h"
#include "CNvSDKInterface.h"
#include "CCommon.h"
#include "CServerDriver.h"
#include "CCameraDriver.h"

extern char g_modulePath[];

char *g_nvARSDKPath = nullptr;

CNvSDKInterface::CNvSDKInterface() : m_tmpImage{}
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
    m_fps = 1;
    driver = nullptr;
    ready = false;
    confidenceRequirement = 0.0;
    m_axisScale = glm::vec3(1.f, 1.f, 1.f);
    m_offset = glm::vec3(0.f, 0.f, 0.f);
    m_alignHMD = true;
}

void CNvSDKInterface::KeyInfoUpdated(bool override)
{
    int _nkp = m_numKeyPoints;

    vr_log("Key information has been update, reloading data in the NVIDIA AR SDK\n");

    NvAR_Create(NvAR_Feature_BodyPoseEstimation, &m_keyPointDetectHandle);

    NvAR_SetString(m_keyPointDetectHandle, NvAR_Parameter_Config(ModelDir), "C:\\Program Files\\NVIDIA Corporation\\NVIDIA AR SDK\\models");
    NvAR_SetCudaStream(m_keyPointDetectHandle, NvAR_Parameter_Config(CUDAStream), m_stream);
    NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(BatchSize), batchSize);
    NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(Mode), nvARMode);
    NvAR_SetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(Temporal), stabilization);
    NvAR_SetF32(m_keyPointDetectHandle, NvAR_Parameter_Config(FocalLength), focalLength);
    NvAR_SetF32(m_keyPointDetectHandle, NvAR_Parameter_Config(UseCudaGraph), useCudaGraph);

    NvAR_GetU32(m_keyPointDetectHandle, NvAR_Parameter_Config(NumKeyPoints), &m_numKeyPoints);

    vr_log("Number of keypoints: %d\n", m_numKeyPoints);

    m_keypoints.assign(realBatches * batchSize * m_numKeyPoints, { 0.f, 0.f });
    m_keypoints3D.assign(realBatches * batchSize * m_numKeyPoints, { 0.f, 0.f, 0.f });
    m_jointAngles.assign(realBatches * batchSize * m_numKeyPoints, { 0.f, 0.f, 0.f, 1.f });
    m_keypointsConfidence.assign(realBatches * batchSize * m_numKeyPoints, 0.f);
    m_referencePose.assign(m_numKeyPoints, { 0.f, 0.f, 0.f });

    m_realKeypoints3D.assign(m_numKeyPoints, { 0.f, 0.f, 0.f });
    m_realJointAngles.assign(m_numKeyPoints, { 0.f, 0.f, 0.f, 0.f });
    m_realConfidence.assign(m_numKeyPoints, 0.f);

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
        m_keypointsConfidence.data(), realBatches * batchSize * m_numKeyPoints);

    NvAR_Load(m_keyPointDetectHandle);

    m_batchSize = batchSize;
}

void CNvSDKInterface::Initialize()
{
    unsigned int output_bbox_size;
    NvAR_CudaStreamCreate(&m_stream);

    KeyInfoUpdated(true);

    output_bbox_size = batchSize;
    if(!stabilization) output_bbox_size = 25;
    m_outputBBoxData.assign(output_bbox_size, { 0.f, 0.f, 0.f, 0.f });
    m_outputBBoxes.boxes = m_outputBBoxData.data();
    m_outputBBoxes.max_boxes = (uint8_t)output_bbox_size;
    m_outputBBoxes.num_boxes = (uint8_t)output_bbox_size;
    NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Output(BoundingBoxes), &m_outputBBoxes, sizeof(NvAR_BBoxes));
}

void CNvSDKInterface::Initialize(int w, int h, int batch_size)
{
    batchSize = batch_size;
    ResizeImage(w, h);
    Initialize();
}

void CNvSDKInterface::ResizeImage(int w, int h)
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

void CNvSDKInterface::LoadImageFromCam(const cv::VideoCapture &cam)
{
    CCameraDriver *camDriv = driver->m_cameraDriver;
    if (m_imageLoaded)
        NvCVImage_Dealloc(&m_inputImageBuffer);
    KeyInfoUpdated(true);
    m_inputImageWidth = camDriv->GetScaledWidth();
    m_inputImageHeight = camDriv->GetScaledHeight();
    m_inputImagePitch = 3 * m_inputImageWidth * sizeof(unsigned char);
    NvCVImage_Alloc(&m_inputImageBuffer, m_inputImageWidth, m_inputImageHeight, NVCV_BGR, NVCV_U8,
        NVCV_CHUNKY, NVCV_GPU, 1);
    NvAR_SetObject(m_keyPointDetectHandle, NvAR_Parameter_Input(Image), &m_inputImageBuffer, sizeof(NvCVImage));
    m_imageLoaded = true;
    ready = true;
}

void CNvSDKInterface::UpdateImageFromCam(const cv::Mat image)
{
    NvCVImage fxSrcChunkyCPU{};
    (void)NVWrapperForCVMat(&image, &fxSrcChunkyCPU);
    NvCVImage_Transfer(&fxSrcChunkyCPU, &m_inputImageBuffer, 1.f, m_stream, &m_tmpImage);
}

CNvSDKInterface::~CNvSDKInterface()
{
    Cleanup();
}

void CNvSDKInterface::Cleanup()
{
    if(m_keyPointDetectHandle != nullptr)
    {
        NvAR_Destroy(m_keyPointDetectHandle);
        m_keyPointDetectHandle = nullptr;
    }
    if(m_stream != nullptr)
    {
        NvAR_CudaStreamDestroy(m_stream);
        m_stream = nullptr;
    }
    if(m_imageLoaded)
        NvCVImage_Dealloc(&m_inputImageBuffer);
}


void CNvSDKInterface::FillBatched(const std::vector<float> &from, std::vector<float> &to)
{
    int index, batch;
    for (index = 0; index < (int)m_numKeyPoints; index++)
    {
        to[index] = 0.f;
        for (batch = 0; batch < realBatches * batchSize; batch++)
        {
            to[index] += TableIndex(from, index, batch);
        }
        to[index] /= (float)(realBatches * batchSize);
    }
}

void CNvSDKInterface::FillBatched(const std::vector<NvAR_Quaternion> &from, std::vector<glm::quat> &to)
{
    int index, batch;
    for(index = 0; index < (int)m_numKeyPoints; index++)
    {
        to[index] = CastQuaternion(from[index]);
    }
    if (realBatches * batchSize <= 1) return;
    for (index = 0; index < (int)m_numKeyPoints; index++)
    {
        for(batch = 1; batch < realBatches * batchSize; batch++)
        {

            to[index] = SumQuat(to[index], CastQuaternion(TableIndex(from, index, batch)));
        }
        to[index] = AvgQuat(to[index], realBatches * batchSize);
    }
}

void CNvSDKInterface::FillBatched(const std::vector<NvAR_Point3f> &from, std::vector<glm::vec3> &to)
{
    int index, batch;
    for (index = 0; index < (int)m_numKeyPoints; index++)
    {
        to[index] = glm::vec3(0.f, 0.f, 0.f);
        for(batch = 0; batch < realBatches * batchSize; batch++)
        {
            to[index] += CastPoint(TableIndex(from, index, batch));
        }
        to[index] /= (float)(realBatches * batchSize);
        //to[index] = CamToWorld(to[index]);
        to[index] *= m_axisScale;
    }
}

void CNvSDKInterface::AlignToHMD(const vr::TrackedDevicePose_t &pose)
{
    if (not pose.bDeviceIsConnected || not pose.bPoseIsValid) return;
    const vr::HmdMatrix34_t &mat = pose.mDeviceToAbsoluteTracking;
    glm::vec3 hmdPosition = WorldToCam(
        glm::vec3(
            mat.m[0][3],
            mat.m[1][3],
            mat.m[2][3]
        )
    );
    glm::vec3 eyes = GetPosition(BODY_JOINT::LEFT_EYE, BODY_JOINT::RIGHT_EYE);
    glm::vec3 offset = hmdPosition - eyes;
    offset += m_offset;
    int index;
    for (index = 0; index < (int)m_numKeyPoints; index++)
    {
        m_realKeypoints3D[index] += offset;
    }
}

void CNvSDKInterface::RotateBatched(std::vector<float> &to)
{
    std::rotate(to.rbegin(), to.rbegin() + m_numKeyPoints, to.rend());
}

void CNvSDKInterface::RotateBatched(std::vector<NvAR_Point3f> &to)
{
    std::rotate(to.rbegin(), to.rbegin() + m_numKeyPoints, to.rend());
}

void CNvSDKInterface::RotateBatched(std::vector<NvAR_Quaternion> &to)
{
    std::rotate(to.rbegin(), to.rbegin() + m_numKeyPoints, to.rend());
}


void CNvSDKInterface::ComputeAvgConfidence()
{
    float avg = 0.0;
    int batch, index;
    for(batch = 0; batch < realBatches * batchSize; batch++)
    {
        for(index = 0; index < (int)m_numKeyPoints; index++)
        {
            avg += TableIndex(m_keypointsConfidence, index, batch);
        }
    }
    m_confidence = avg / (realBatches * batchSize * m_numKeyPoints);
}

void CNvSDKInterface::EmptyKeypoints()
{
    m_realKeypoints3D.assign(m_numKeyPoints, { 0.f, 0.f, 0.f });
    m_realJointAngles.assign(m_numKeyPoints, { 0.f, 0.f, 0.f, 0.f });
    m_realConfidence.assign(m_numKeyPoints, 0.f);
}

void CNvSDKInterface::DebugSequence(const std::vector<float> conf) const
{
    uint counter = 0;
    for (auto &val : conf)
    {
        vr_log("\t%d: JOINT %s CONFIDENCE %.3f", counter / m_numKeyPoints, BodyJointName[counter % m_numKeyPoints], val);
        counter++;
    }
    vr_log("");
}
void CNvSDKInterface::DebugSequence(const std::vector<NvAR_Point3f> kep) const
{
    uint counter = 0;
    for (auto &val : kep)
    {
        vr_log("\t%d: JOINT %s KEYPOINT < %.3f %.3f %.3f >", counter / m_numKeyPoints, BodyJointName[counter % m_numKeyPoints], val.x, val.y, val.z);
        counter++;
    }
    vr_log("");
}
void CNvSDKInterface::DebugSequence(const std::vector<NvAR_Quaternion> kep) const
{
    uint counter = 0;
    for (auto &val : kep)
    {
        vr_log("\t%d: JOINT %s ANGLES < %.3f %.3f %.3f %.3f >", counter / m_numKeyPoints, BodyJointName[counter % m_numKeyPoints], val.w, val.x, val.y, val.z);
        counter++;
    }
    vr_log("");
}
void CNvSDKInterface::DebugSequence(const std::vector<glm::vec3> kep) const
{
    uint counter = 0;
    for (auto &val : kep)
    {
        vr_log("\t%d: JOINT %s KEYPOINT < %.3f %.3f %.3f >", counter / m_numKeyPoints, BodyJointName[counter % m_numKeyPoints], val.x, val.y, val.z);
        counter++;
    }
    vr_log("");
}
void CNvSDKInterface::DebugSequence(const std::vector<glm::quat> rot) const
{
    uint counter = 0;
    for (auto &val : rot)
    {
        vr_log("\t%d: JOINT %s ANGLES < %.3f %.3f %.3f %.3f >", counter / m_numKeyPoints, BodyJointName[counter % m_numKeyPoints], val.w, val.x, val.y, val.z);
        counter++;
    }
    vr_log("");
}

void CNvSDKInterface::RunFrame()
{
    if(trackingActive)
    {
        //NvAR_Run(m_keyPointDetectHandle);
        batchSize = 1;
        int code;
        for (int i = 0; i < realBatches; i++)
        {
            if (i > 0)
            {
                RotateBatched(m_keypointsConfidence);
                RotateBatched(m_keypoints3D);
                //RotateBatched(m_jointAngles);
            }
            code = (int)NvAR_Run(m_keyPointDetectHandle);
            if (code != 0) vr_log("NVIDIA SDK ERR CODE:\t%d", code);
        }
        ComputeAvgConfidence();
        //vr_log("CONFIDENCE: %.5f", m_confidence);
        if(m_confidence >= confidenceRequirement)
        {
            FillBatched(m_keypointsConfidence, m_realConfidence);
            FillBatched(m_keypoints3D, m_realKeypoints3D);
            //FillBatched(m_jointAngles, m_realJointAngles);
            if (m_alignHMD) AlignToHMD(driver->m_hmd_controller_pose[0]);
            //DebugSequence(m_keypoints3D);
        }
        else
            EmptyKeypoints();
    }
    else
        EmptyKeypoints();
}