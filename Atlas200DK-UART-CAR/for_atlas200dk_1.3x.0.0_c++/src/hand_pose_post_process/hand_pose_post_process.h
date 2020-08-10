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
#ifndef HAND_POSE_POST_PROCESS_H_
#define HAND_POSE_POST_PROCESS_H_
#include "hand_pose_params.h"
#include "hiaiengine/api.h"
#include "hiaiengine/ai_types.h"
#include "hiaiengine/data_type.h"
#include "hiaiengine/data_type_reg.h"
#include "hiaiengine/engine.h"
#include "ascenddk/presenter/agent/presenter_channel.h"
#include "uart.h"

#define INPUT_SIZE 1
#define OUTPUT_SIZE 1

// hand pose configuration
struct HandPosePostConfig {
  float confidence;  // confidence
  std::string presenter_ip;  // presenter server IP
  int32_t presenter_port;  // presenter server port for agent
  std::string channel_name;  // channel name
};

/**
 * @brief: hand pose post process
 */
class HandPosePostProcess : public hiai::Engine {
public:

  /**
   * @brief: initialize UART
   */
  uart ctrl_uart;


  /**
   * @brief: construction function
   */
  HandPosePostProcess();

  /**
   * @brief: the destruction function
   */
  ~HandPosePostProcess() = default;

  /**
   * @brief: hand pose post process engine initialize
   * @param [in]: engine's parameters which configured in graph.config
   * @param [in]: model description
   * @return: HIAI_StatusT
   */
  HIAI_StatusT Init(const hiai::AIConfig& config,
                    const std::vector<hiai::AIModelDescription>& model_desc);

  /**
   * @brief: engine processor
   *         1. dealing results
   *         2. call OSD to draw box and label if needed
   *         3. call DVPP to change YUV420SP to JPEG
   *         4. call presenter agent to send JPEG to server
   * @param [in]: input size
   * @param [in]: output size
   */
  HIAI_DEFINE_PROCESS(INPUT_SIZE, OUTPUT_SIZE);

private:
  // configuration
  std::shared_ptr<HandPosePostConfig> fd_post_process_config_;

  // presenter channel
  std::shared_ptr<ascend::presenter::Channel> presenter_channel_;

  /**
   * @brief: handle original image
   * @param [in]: EngineTransT format data which inference engine send
   * @return: HIAI_StatusT
   */
  HIAI_StatusT HandleOriginalImage(
      const std::shared_ptr<EngineTransT> &inference_res);

  /**
   * @brief: handle results
   * @param [in]: EngineTransT format data which inference engine send
   * @return: HIAI_StatusT
   */
  HIAI_StatusT HandleResults(
      const std::shared_ptr<EngineTransT> &inference_res);

  /**
   * @brief: validate IP address
   * @param [in]: IP address
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidIp(const std::string &ip);

  /**
   * @brief: validate port
   * @param [in]: port
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidPort(int32_t port);

  /**
   * @brief: validate channel name
   * @param [in]: channel name
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidChannelName(const std::string &channel__name);

  /**
   * @brief: convert YUV420SP to JPEG, and then send to presenter
   * @param [in]: image height
   * @param [in]: image width
   * @param [in]: image size
   * @param [in]: image data
   * @return: FD_FUN_FAILED or FD_FUN_SUCCESS
   */
  int32_t SendImage(uint32_t height, uint32_t width, uint32_t size,
                    u_int8_t *data, vector<ascend::presenter::DetectionResult>& detection_results);

};

/**
 * @brief: compute RC car command
 */
class Handpose_result{
public:
    int x_arr [21];
    int y_arr[21];
    const int x_max_threshold = 1200;
    const int x_min_threshold = 50;
    const int thumb_threshold = 60;
    const unsigned char cmd_stop = 0;
    const unsigned char cmd_forward = 1;
    const unsigned char cmd_backward = 2;
    const unsigned char cmd_left = 3;
    const unsigned char cmd_right = 4;
    const int left_thresh = -700;
    const int right_thresh = 500;

    /**
    * @brief: validate keypoints
    * @return: true: valid
    *          false: invalid
    */
    int validate(){
        if((max(x_arr) > x_max_threshold) || (min(x_arr) < x_min_threshold)){
            return 0;
        }
        else if(min_index(y_arr) != 8 && min_index(y_arr) != 12 && min_index(y_arr) != 16 && min_index(y_arr) != 20){
            return 0;
        }
        return 1;
    }

    /**
    * @brief: calculate top of finger keypoint relative to bottom
    * @return: value proportional to angle of fingers
    */
    int get_fingers_angle_scale(){
        int finger_top_total = x_arr[8] + x_arr[12] + x_arr[16] + x_arr[20];
        int finger_bottom_total = x_arr[5] + x_arr[9] + x_arr[13] + x_arr[17];
        int angle_scale = finger_top_total - finger_bottom_total;
        return angle_scale;
    }

    /**
    * @brief: thumb pointing inward or outward
    * @return: true: inward
    *          false: outward
    */
    int thumb_status(){
        int x_diff = x_arr[4] - x_arr[5];
        if(x_diff < thumb_threshold){
            return 1;
        }
        else {
            return 0;
        }
    }

    /**
    * @brief: compute rc_command based on keypoint coordinates
    * @return: 0: stop
    *          1: forward
    *          2: backward
    *          3: left
    *          4: right
    */
    unsigned char get_rc_command(){
        int angle = get_fingers_angle_scale();
        int thumb_bool = thumb_status();
        if(!validate()){
            return cmd_stop;
        }
        else if((left_thresh < angle < right_thresh) && thumb_bool){
            return cmd_backward;
        }
        else if(angle > right_thresh){
            return cmd_right;
        }
        else if(angle < left_thresh){
            return cmd_left;
        }
        else {
            return cmd_forward;
        }
    }

private:
    /**
    * @brief: find index of minimum value in array
    */
    int min_index(int arr[]){
        int min_index = 0;
        for(int k=0; k<21; k++){
            if(arr[k] < arr[min_index]){
                min_index = k;
            }
        }
        return min_index;
    }

    /**
    * @brief: find maximum value in array
    */
    int max(int arr[]){
        int max = 0;
        for(int k=0; k<21; k++){
            if(arr[k] > max){
                max = arr[k];
            }
        }
        return max;
    }

    /**
    * @brief: find minimum value in array
    */
    int min(int arr[]){
        int min = 1280;
        for(int k=0; k<21; k++){
            if(arr[k] < min){
                min = arr[k];
            }
        }
        return min;
    }
};





#endif /* HAND_POSE_POST_PROCESS_H_ */
