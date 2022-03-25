#include "pch.h"
#include "CBodyTrackDriver.h"
#include "Common.h"

char* g_nvARSDKPath = NULL;

CBodyTrackDriver::CBodyTrackDriver()
{
	trackingActive = false;
	stabilization = true;
	image_loaded = false;
	useCudaGraph = true;
	nvARMode = 1;
	focalLength = 800.0f;
	batchSize = 1;
	_batchSize = 1;
}

void CBodyTrackDriver::KeyInfoUpdated()
{
	int _nkp = numKeyPoints;

	if (batchSize != _batchSize)
	{
		_batchSize = batchSize;
		if (keyPointDetectHandle != nullptr)
		{
			NvAR_Destroy(keyPointDetectHandle);
			keyPointDetectHandle = nullptr;
		}
		NvAR_Create(NvAR_Feature_BodyPoseEstimation, &keyPointDetectHandle);
		NvAR_SetString(keyPointDetectHandle, NvAR_Parameter_Config(ModelDir), NULL);
		NvAR_SetCudaStream(keyPointDetectHandle, NvAR_Parameter_Config(CUDAStream), stream);
		NvAR_SetU32(keyPointDetectHandle, NvAR_Parameter_Config(BatchSize), batchSize);
	}
	NvAR_SetU32(keyPointDetectHandle, NvAR_Parameter_Config(Mode), nvARMode);
	NvAR_SetU32(keyPointDetectHandle, NvAR_Parameter_Config(Temporal), stabilization);
	NvAR_SetF32(keyPointDetectHandle, NvAR_Parameter_Config(FocalLength), focalLength);
	NvAR_SetF32(keyPointDetectHandle, NvAR_Parameter_Config(UseCudaGraph), useCudaGraph);
	NvAR_Load(keyPointDetectHandle);

	NvAR_GetU32(keyPointDetectHandle, NvAR_Parameter_Config(NumKeyPoints), &numKeyPoints);
	if (batchSize != _batchSize || numKeyPoints != _nkp)
	{
		keypoints.assign(batchSize * numKeyPoints, { 0.f, 0.f });
		keypoints3D.assign(batchSize * numKeyPoints, { 0.f, 0.f, 0.f });
		jointAngles.assign(batchSize * numKeyPoints, { 0.f, 0.f, 0.f, 1.f });
		keypoints_confidence.assign(batchSize * numKeyPoints, 0.f);
		referencePose.assign(numKeyPoints, { 0.f, 0.f, 0.f });

		EmptyKeypoints();

		const void* pReferencePose;
		NvAR_GetObject(keyPointDetectHandle, NvAR_Parameter_Config(ReferencePose), &pReferencePose,
			sizeof(NvAR_Point3f));
		memcpy(referencePose.data(), pReferencePose, sizeof(NvAR_Point3f) * numKeyPoints);

		NvAR_SetObject(keyPointDetectHandle, NvAR_Parameter_Output(KeyPoints), keypoints.data(),
			sizeof(NvAR_Point2f));
		NvAR_SetObject(keyPointDetectHandle, NvAR_Parameter_Output(KeyPoints3D), keypoints3D.data(),
			sizeof(NvAR_Point3f));
		NvAR_SetObject(keyPointDetectHandle, NvAR_Parameter_Output(JointAngles), jointAngles.data(),
			sizeof(NvAR_Quaternion));
		NvAR_SetF32Array(keyPointDetectHandle, NvAR_Parameter_Output(KeyPointsConfidence),
			keypoints_confidence.data(), batchSize * numKeyPoints);
	}
}

void CBodyTrackDriver::Initialize()
{
	if (stream != NULL)
		Cleanup();

	unsigned int output_bbox_size;
	NvAR_CudaStreamCreate(&stream);
	NvAR_Create(NvAR_Feature_BodyDetection, &bodyDetectHandle);
	NvAR_SetString(bodyDetectHandle, NvAR_Parameter_Config(ModelDir), NULL);
	NvAR_SetCudaStream(bodyDetectHandle, NvAR_Parameter_Config(CUDAStream), stream);
	NvAR_SetU32(bodyDetectHandle, NvAR_Parameter_Config(Temporal), stabilization);
	NvAR_Load(bodyDetectHandle);

	KeyInfoUpdated();

	output_bbox_size = batchSize;
	if (!stabilization) output_bbox_size = 25;
	output_bbox_data.assign(output_bbox_size, { 0.f, 0.f, 0.f, 0.f });
	output_bboxes.boxes = output_bbox_data.data();
	output_bboxes.max_boxes = (uint8_t)output_bbox_size;
	output_bboxes.num_boxes = (uint8_t)output_bbox_size;
	NvAR_SetObject(keyPointDetectHandle, NvAR_Parameter_Output(BoundingBoxes), &output_bboxes, sizeof(NvAR_BBoxes));
}

void CBodyTrackDriver::Initialize(int w, int h, int batch_size)
{
	batchSize = batch_size;
	ResizeImage(w, h);
	Initialize();
}

void CBodyTrackDriver::ResizeImage(int w, int h)
{
	input_image_width = w;
	input_image_height = h;
	input_image_pitch = 3 * input_image_width * sizeof(unsigned char);

	if (image_loaded)
		NvCVImage_Dealloc(&inputImageBuffer);
	NvCVImage_Alloc(&inputImageBuffer, input_image_width, input_image_height, NVCV_BGR, NVCV_U8,
		NVCV_CHUNKY, NVCV_GPU, 1);
	NvAR_SetObject(keyPointDetectHandle, NvAR_Parameter_Input(Image), &inputImageBuffer, sizeof(NvCVImage));
	image_loaded = true;
}

CBodyTrackDriver::~CBodyTrackDriver()
{
	Cleanup();
}

void CBodyTrackDriver::Cleanup()
{
	if (keyPointDetectHandle != nullptr)
	{
		NvAR_Destroy(keyPointDetectHandle);
		keyPointDetectHandle = nullptr;
	}
	if (bodyDetectHandle != nullptr)
	{
		NvAR_Destroy(bodyDetectHandle);
		bodyDetectHandle = nullptr;
	}
	if (stream != NULL)
	{
		NvAR_CudaStreamDestroy(stream);
		stream = NULL;
	}
	if (image_loaded)
		NvCVImage_Dealloc(&inputImageBuffer);
}

void CBodyTrackDriver::FillBatched(std::vector<NvAR_Quaternion>& from, std::vector<glm::quat>& to)
{
	int index, batch;
	for (index = 0; index < (int)numKeyPoints; index++)
	{
		to[index] = CastQuaternion(from[index]) / (float)batchSize;
	}

	for (batch = 1; batch < batchSize; batch++)
	{
		for (index = 0; index < (int)numKeyPoints; index++)
		{
			to[index] += CastQuaternion(TableIndex(from, index, batch)) / (float)batchSize;
		}
	}
}

void CBodyTrackDriver::FillBatched(std::vector<NvAR_Point3f>& from, std::vector<glm::vec3>& to)
{
	int index, batch;
	for (index = 0; index < (int)numKeyPoints; index++)
	{
		to[index] = CastPoint(from[index]) / (float)batchSize;
	}

	for (batch = 1; batch < batchSize; batch++)
	{
		for (index = 0; index < (int)numKeyPoints; index++)
		{
			to[index] += CastPoint(TableIndex(from, index, batch)) / (float)batchSize;
		}
	}
}

void CBodyTrackDriver::ComputeAvgConfidence()
{
	float avg = 0.0;
	int batch, index;
	for (batch = 0; batch < batchSize; batch++)
	{
		for (index = 0; index < (int)numKeyPoints; index++)
		{
			avg += TableIndex(keypoints_confidence, index, batch);
		}
	}
	confidence = avg / (batchSize + numKeyPoints);
}

void CBodyTrackDriver::EmptyKeypoints()
{
	real_keypoints3D.assign(numKeyPoints, { 0.f, 0.f, 0.f });
	real_jointAngles.assign(numKeyPoints, { 0.f, 0.f, 0.f, 0.f });
}

void CBodyTrackDriver::RunFrame()
{
	if (trackingActive)
	{
		NvAR_Run(keyPointDetectHandle);
		ComputeAvgConfidence();
		if (confidence >= confidenceRequirement)
		{
			FillBatched(keypoints3D, real_keypoints3D);
			FillBatched(jointAngles, real_jointAngles);
		}
		else
			EmptyKeypoints();
	}
	else
		EmptyKeypoints();
}