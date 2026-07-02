/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef CLASSIFICATION_INFERENCE_HPP_
#define CLASSIFICATION_INFERENCE_HPP_

#include <opencv2/opencv.hpp>
#include <chrono>
#include <filesystem>
#include "model.hpp"
#include <vector>
#include <stdexcept>
#include <string>
#include <opencv2/dnn.hpp> // 需要包含 dnn 模块用于 NMS
#include <cmath>
#include <algorithm>
#include "terminator/terminator.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qrb
{
namespace vla
{
class ClassificationInference
{
private:
    void preprocess_convnext(const cv::Mat& img_bgr, std::vector<float>& nchw_float);
    int postprocess_binary(const std::vector<std::vector<float>>& output_data, float& prob0, float& prob1);
    std::vector<float> softmax(const std::vector<float>& logits);

    std::shared_ptr<Terminator> terminator_;
    rclcpp::Logger logger_{rclcpp::get_logger("classification_inference")};

public:
  ClassificationInference(std::shared_ptr<Terminator>& terminator);
  ~ClassificationInference();

  int inference(cv::Mat& img_bgr);
  void inference(cv::Mat& img_bgr, uint32_t dir_idx);
};
}  // namespace vla
}  // namespace qrb
#endif // CLASSIFICATION_INFERENCE_HPP_