/**
* Copyright 2020 Huawei Technologies Co., Ltd
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.

* File utils.h
* Description: handle file operations
*/
#pragma once
#include <iostream>
#include <vector>
#include "acl/acl.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/legacy/constants_c.h"
#include "opencv2/imgproc/types_c.h"
#include "ascenddk/presenter/agent/presenter_channel.h"

#define INFO_LOG(fmt, args...) fprintf(stdout, "[INFO]  " fmt "\n", ##args)
#define WARN_LOG(fmt, args...) fprintf(stdout, "[WARN]  " fmt "\n", ##args)
#define ERROR_LOG(fmt, args...) fprintf(stdout, "[ERROR]  " fmt "\n", ##args)
using namespace ascend::presenter;

#define MODEL_INPUT_WIDTH	416
#define MODEL_INPUT_HEIGHT	416


typedef enum Result {
    SUCCESS = 0,
    FAILED = 1
} Result;

typedef struct PicDesc {
    std::string picName;
    int width;
    int height;
}PicDesc;

struct ConsoleParams {
  uint32_t img_width = 0;
  uint32_t img_height = 0;
  int32_t size = 0;
  std::string input_path = "";
  std::shared_ptr<u_int8_t> data;
};

const std::string kImagePathSeparator = ",";
const int kStatSuccess = 0;
const std::string kFileSperator = "/";
const std::string kPathSeparator = "/";
// output image prefix
const std::string kOutputFilePrefix = "out_";

/**
 * Utils
 */
class Utils {
public:

    /**
    * @brief create device buffer of pic
    * @param [in] picDesc: pic desc
    * @param [in] PicBufferSize: aligned pic size
    * @return device buffer of pic
    */

    static Result PresentOutputFrame(Channel *channel, aclmdlDataset *modelOutput, cv::Mat mat);

    static Result SaveDvppOutputData(const char *fileName, const void *devPtr, uint32_t dataSize);

    static bool IsDirectory(const std::string &path);

    static bool IsPathExist(const std::string &path);

    static void SplitPath(const std::string &path, std::vector<std::string> &path_vec);

    static void GetAllFiles(const std::string &path, std::vector<std::string> &file_vec);

    static void GetPathFiles(const std::string &path, std::vector<std::string> &file_vec);

};

