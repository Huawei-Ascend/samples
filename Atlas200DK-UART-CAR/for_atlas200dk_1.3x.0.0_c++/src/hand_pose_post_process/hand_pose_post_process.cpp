/**
 * ============================================================================
 *
 * Copyright (C) 2018, Hisilicon Technologies Co., Ltd. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1 Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   2 Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   3 Neither the names of the copyright holders nor the names of the
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */
#include "hand_pose_post_process.h"
#include <cstdlib>
#include <vector>
#include <sstream>
#include <cmath>
#include <regex>
#include "hiaiengine/log.h"


using hiai::Engine;
using namespace ascend::presenter;
using namespace std::__cxx11;

// register data type
HIAI_REGISTER_DATA_TYPE("EngineTransT", EngineTransT);
HIAI_REGISTER_DATA_TYPE("OutputT", OutputT);
HIAI_REGISTER_DATA_TYPE("ScaleInfoT", ScaleInfoT);
HIAI_REGISTER_DATA_TYPE("NewImageParaT", NewImageParaT);
HIAI_REGISTER_DATA_TYPE("BatchImageParaWithScaleT", BatchImageParaWithScaleT);

// constants
namespace {
// scale of camera image relative to inference output
const float widthScale = 1280 / 64;
const float heightScale =  720 / 64;

// data to send through UART 通过UART发送的数据
char command_char;

int command_char1;

// port number range
const int32_t kPortMinNumber = 0;
const int32_t kPortMaxNumber = 65535;


// hand pose function return value
const int32_t kFdFunSuccess = 0;
const int32_t kFdFunFailed = -1;

// need to deal results when index is 2
const int32_t kDealResultIndex = 2;

// each results size
const int32_t kEachResultSize = 7;

// percent
const int32_t kScorePercent = 100;

// IP regular expression
const std::string kIpRegularExpression =
    "^((25[0-5]|2[0-4]\\d|[1]{1}\\d{1}\\d{1}|[1-9]{1}\\d{1}|\\d{1})($|(?!\\.$)\\.)){4}$";

// channel name regular expression
const std::string kChannelNameRegularExpression = "[a-zA-Z0-9/]+";
}

HandPosePostProcess::HandPosePostProcess() {
  fd_post_process_config_ = nullptr;
  presenter_channel_ = nullptr;

}

HIAI_StatusT HandPosePostProcess::Init(
    const hiai::AIConfig& config,
    const std::vector<hiai::AIModelDescription>& model_desc) {
  HIAI_ENGINE_LOG("Begin initialize!");


      // open UART with correct baudrate (38400)
  	  if(ctrl_uart.uart_open() < 0)
      {
        perror("uart_open error");
      }
      if(ctrl_uart.uart_set_option(9600,8,'N',1) < 0) //9600 38400
      {
        perror("uart_set_option error");
      }

  // get configurations
  if (fd_post_process_config_ == nullptr) {
    fd_post_process_config_ = std::make_shared<HandPosePostConfig>();
  }

  // get parameters from graph.config
  for (int index = 0; index < config.items_size(); index++) {
    const ::hiai::AIConfigItem& item = config.items(index);
    const std::string& name = item.name();
    const std::string& value = item.value();
    std::stringstream ss;
    ss << value;
    if (name == "PresenterIp") {
      // validate presenter server IP
      if (IsInValidIp(value)) {
        HIAI_ENGINE_LOG(HIAI_GRAPH_INVALID_VALUE,
                        "PresenterIp=%s which configured is invalid.",
                        value.c_str());
        return HIAI_ERROR;
      }
      ss >> (*fd_post_process_config_).presenter_ip;
    } else if (name == "PresenterPort") {
      ss >> (*fd_post_process_config_).presenter_port;
      // validate presenter server port
      if (IsInValidPort(fd_post_process_config_->presenter_port)) {
        HIAI_ENGINE_LOG(HIAI_GRAPH_INVALID_VALUE,
                        "PresenterPort=%s which configured is invalid.",
                        value.c_str());
        return HIAI_ERROR;
      }
    } else if (name == "ChannelName") {
      // validate channel name
      if (IsInValidChannelName(value)) {
        HIAI_ENGINE_LOG(HIAI_GRAPH_INVALID_VALUE,
                        "ChannelName=%s which configured is invalid.",
                        value.c_str());
        return HIAI_ERROR;
      }
      ss >> (*fd_post_process_config_).channel_name;
    }
    // else : nothing need to do
  }

  // call presenter agent, create connection to presenter server
  uint16_t u_port = static_cast<uint16_t>(fd_post_process_config_
      ->presenter_port);
  OpenChannelParam channel_param = { fd_post_process_config_->presenter_ip,
      u_port, fd_post_process_config_->channel_name, ContentType::kVideo };
  Channel *chan = nullptr;
  PresenterErrorCode err_code = OpenChannel(chan, channel_param);
  // open channel failed
  if (err_code != PresenterErrorCode::kNone) {
    HIAI_ENGINE_LOG(HIAI_GRAPH_INIT_FAILED,
                    "Open presenter channel failed, error code=%d", err_code);
    return HIAI_ERROR;
  }

  presenter_channel_.reset(chan);
  HIAI_ENGINE_LOG(HIAI_DEBUG_INFO, "End initialize!");
  return HIAI_OK;
}

bool HandPosePostProcess::IsInValidIp(const std::string &ip) {
  regex re(kIpRegularExpression);
  smatch sm;
  return !regex_match(ip, sm, re);
}

bool HandPosePostProcess::IsInValidPort(int32_t port) {
  return (port <= kPortMinNumber) || (port > kPortMaxNumber);
}

bool HandPosePostProcess::IsInValidChannelName(
    const std::string &channel_name) {
  regex re(kChannelNameRegularExpression);
  smatch sm;
  return !regex_match(channel_name, sm, re);
}


int32_t HandPosePostProcess::SendImage(uint32_t height, uint32_t width,
                                            uint32_t size, u_int8_t *data, std::vector<DetectionResult>& detection_results) {
  int32_t status = kFdFunSuccess;
  // parameter
  ImageFrame image_frame_para;
  image_frame_para.format = ImageFormat::kJpeg;
  image_frame_para.width = width;
  image_frame_para.height = height;
  image_frame_para.size = size;
  image_frame_para.data = data;
  image_frame_para.detection_results = detection_results;

  PresenterErrorCode p_ret = PresentImage(presenter_channel_.get(),
                                            image_frame_para);
  // send to presenter failed
  if (p_ret != PresenterErrorCode::kNone) {
    HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
                      "Send JPEG image to presenter failed, error code=%d",
                      p_ret);
    status = kFdFunFailed;
  }

  return status;
}

HIAI_StatusT HandPosePostProcess::HandleOriginalImage(
    const std::shared_ptr<EngineTransT> &inference_res) {
  HIAI_StatusT status = HIAI_OK;
  std::vector<NewImageParaT> img_vec = inference_res->imgs;
  // dealing every original image
  for (uint32_t ind = 0; ind < inference_res->b_info.batch_size; ind++) {
    uint32_t width = img_vec[ind].img.width;
    uint32_t height = img_vec[ind].img.height;
    uint32_t size = img_vec[ind].img.size;

    // call SendImage
    // 1. call DVPP to change YUV420SP image to JPEG
    // 2. send image to presenter
    vector<DetectionResult> detection_results;
    int32_t ret = SendImage(height, width, size, img_vec[ind].img.data.get(), detection_results);
    if (ret == kFdFunFailed) {
      status = HIAI_ERROR;
      continue;
    }
  }
  return status;
}

HIAI_StatusT HandPosePostProcess::HandleResults(
    const std::shared_ptr<EngineTransT> &inference_res) {
  HIAI_StatusT status = HIAI_OK;
  std::vector<NewImageParaT> img_vec = inference_res->imgs;
  std::vector<OutputT> output_data_vec = inference_res->output_datas;

   //printf("zhimins %s\n",inference_res->msg.c_str());

  // dealing every image
  for (uint32_t ind = 0; ind < inference_res->b_info.batch_size; ind++) {
    // result
    int32_t out_index = ind * kDealResultIndex;
    OutputT out = output_data_vec[out_index];
    std::shared_ptr<hiai::AISimpleTensor> result_tensor = std::make_shared<
        hiai::AISimpleTensor>();
    result_tensor->SetBuffer(out.data.get(), out.size);
    int32_t size = result_tensor->GetSize() / sizeof(float);
    int result[size];
    errno_t mem_ret = memcpy_s(result, sizeof(result),
                               result_tensor->GetBuffer(),
                               result_tensor->GetSize());
    // memory copy failed, skip this image
    if (mem_ret != EOK) {
      HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
                      "handle results: memcpy_s() error=%d", mem_ret);
      continue;
    }

    uint32_t width = img_vec[ind].img.width;
    uint32_t height = img_vec[ind].img.height;
    uint32_t img_size = img_vec[ind].img.size;

    std::vector<DetectionResult> detection_results;
    DetectionResult one_result;

    // get keypoint coordinates from inference output
    int *ptr = result;
    Handpose_result handpose_result;
    for (int32_t k = 0; k < size; k++) {
      int argmax_index = ptr[k];
      int x_coordinate = argmax_index % 64;
      int y_coordinate = argmax_index / 64;
      x_coordinate = x_coordinate * widthScale;
      y_coordinate = y_coordinate * heightScale;
      handpose_result.x_arr[k] = x_coordinate;
      handpose_result.y_arr[k] = y_coordinate;
    }

    one_result.result_text.append("Command: ");

    // get rc_command index from keypoints and add to presenter server string
    int rc_command = int(handpose_result.get_rc_command());
    switch(rc_command){
        case 0: one_result.result_text.append("STOP"); break;
        case 1: one_result.result_text.append("FORWARD"); break;
        case 2: one_result.result_text.append("BACKWARD"); break;
        case 3: one_result.result_text.append("LEFT"); break;
        case 4: one_result.result_text.append("RIGHT"); break;
    }


    // add keypoint coordinates to presenter server string
    // if command is not "stop"
    if(rc_command != 0){
    one_result.result_text.append("--");  // delimiter to seperate text and coordinates
        for (int32_t k = 0; k < size; k++) {
          one_result.result_text.append(to_string(handpose_result.x_arr[k]));
          one_result.result_text.append(",");
          one_result.result_text.append(to_string(handpose_result.y_arr[k]));
          one_result.result_text.append(" ");
        }
      }
      // left top
      Point point_lt, point_rb;
      point_lt.x = 0;
      point_lt.y = 0;
      // right bottom
      point_rb.x = 0;
      point_rb.y = 0;
      one_result.lt = point_lt;
      one_result.rb = point_rb;

     printf("rc_command %d\n",rc_command);
     if (rc_command == 4){
        command_char = 'r';
     }
     else if(rc_command == 1){
        command_char = 'f';
     }
     else if(rc_command == 2){
        command_char = 'b';
     }
     else if(rc_command == 3){
        command_char = 'l';
     }
     else {
        command_char = 's';
     }

     printf("command_char %c\n",command_char);
     // send command character through UART
     char* send_buf = &command_char;
     printf("send_buf %c\n",*send_buf);
     printf("send_buf %d\n",*send_buf);

     int len = ctrl_uart.uart_send(send_buf, sizeof(command_char)); //date can send success
     printf("uart_send len= %d\n",len);
     if (len < 0)
     {
         printf("UART write data error");
     }


    // push back
    detection_results.emplace_back(one_result);

    // send data to presenter server
    int ret = SendImage(height, width, img_size, img_vec[ind].img.data.get(), detection_results);

    // check send result
    if (ret == kFdFunFailed) {
      status = HIAI_ERROR;
    }
  }
  return status;
}

HIAI_IMPL_ENGINE_PROCESS("hand_pose_post_process",
    HandPosePostProcess, INPUT_SIZE) {
  // check arg0 is null or not
  if (arg0 == nullptr) {
    HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
                    "Failed to process invalid message.");
    return HIAI_ERROR;
  }

  // check original image is empty or not
  std::shared_ptr<EngineTransT> inference_res = std::static_pointer_cast<
      EngineTransT>(arg0);
  if (inference_res->imgs.empty()) {
    HIAI_ENGINE_LOG(
        HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
        "Failed to process invalid message, original image is null.");
    return HIAI_ERROR;
  }

  // inference failed, dealing original images
  if (!inference_res->status) {
    HIAI_ENGINE_LOG(HIAI_OK, inference_res->msg.c_str());
    HIAI_ENGINE_LOG(HIAI_OK, "will handle original image.");
    return HandleOriginalImage(inference_res);
  }

  // inference success, dealing inference results
  return HandleResults(inference_res);
}
