#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <limits>
#include <algorithm>
#include <functional>
#include <future>
#include <map>

#include "openvr_driver.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_interpolation.hpp"
#include "SimpleIni.h"

#include "nvAR.h"
#include "nvAR_defs.h"
#include "nvCVImage.h"
#include "nvCVOpenCV.h"