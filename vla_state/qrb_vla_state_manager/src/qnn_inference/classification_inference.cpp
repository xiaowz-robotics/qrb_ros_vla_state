/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qnn_inference/classification_inference.hpp"

namespace qrb
{
namespace vla
{
ClassificationInference::ClassificationInference(std::shared_ptr<Terminator>& terminator)
: terminator_(terminator)
{
}

ClassificationInference::~ClassificationInference()
{
}

void ClassificationInference::inference(cv::Mat& img_bgr, uint32_t dir_idx)
{
    int result = inference(img_bgr);
    if (result == 1) {
        RCLCPP_INFO(logger_, "++++++++++++++++++++++++++++++++++++++++++++++++++");
        std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Inference result is grap: " << std::endl;
    } else if (result == 0) {
        RCLCPP_INFO(logger_, "---------------------------------------------------");
        std::cout << "---------------------------------------------------------------------------------------------Inference result is songkai: " << std::endl;
    } else {
        std::cout << "Inference result is invalid for dir: " << dir_idx << std::endl;
    }
}

int ClassificationInference::inference(cv::Mat& img_bgr)
{
    std::string model_path = "/home/ubuntu/vla/vla/vla_state/qrb_vla_state_manager/model/libmodel.serialized.bin";
    std::string perf_profile = "burst";

    Model convnext;
    convnext.Init_model(model_path);

    auto start = std::chrono::high_resolution_clock::now();

    // preprocess
    std::vector<float> input_nchw;
    preprocess_convnext(img_bgr, input_nchw);

    // inference
    std::vector<std::vector<float>> output_data;
    convnext.Inference(input_nchw, output_data, perf_profile);

    //postprocess
    float p0, p1;
    int pred = postprocess_binary(output_data, p0, p1);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;

    std::cout << "Pred=" << pred
              << " p0=" << p0
              << " p1=" << p1
              << " time=" << ms.count() << " ms\n";
    RCLCPP_INFO(logger_, "Pred=%d, p0=%.3f, p1=%.3f, time=%.3f ms", pred, p0, p1, ms.count());
    bool moving = (pred == 1)? true : false;
    terminator_->update_manipulation_state(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count(), moving);
    return pred;
}

void ClassificationInference::preprocess_convnext(
    const cv::Mat& img_bgr,
    std::vector<float>& nchw_float   // 输出：1x3x224x224
) {
    if (img_bgr.empty()) {
        throw std::runtime_error("Input image is empty");
    }

    // 1. BGR -> RGB
    cv::Mat img_rgb;
    cv::cvtColor(img_bgr, img_rgb, cv::COLOR_BGR2RGB);

    // 2. Resize to 224x224
    cv::Mat img_resized;
    cv::resize(img_rgb, img_resized, cv::Size(224, 224), 0, 0, cv::INTER_LINEAR);

    // 3. uint8 -> float32, scale to [0,1]
    cv::Mat img_float;
    img_resized.convertTo(img_float, CV_32F, 1.0 / 255.0);

    // 4. Normalize (ImageNet)
    const float mean[3] = {0.485f, 0.456f, 0.406f};
    const float stdv[3] = {0.229f, 0.224f, 0.225f};

    for (int h = 0; h < img_float.rows; ++h) {
        cv::Vec3f* row = img_float.ptr<cv::Vec3f>(h);
        for (int w = 0; w < img_float.cols; ++w) {
            row[w][0] = (row[w][0] - mean[0]) / stdv[0];
            row[w][1] = (row[w][1] - mean[1]) / stdv[1];
            row[w][2] = (row[w][2] - mean[2]) / stdv[2];
        }
    }

    // 5. HWC -> NCHW
    nchw_float.resize(1 * 3 * 224 * 224);
    int idx = 0;
    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < 224; ++h) {
            const cv::Vec3f* row = img_float.ptr<cv::Vec3f>(h);
            for (int w = 0; w < 224; ++w) {
                nchw_float[idx++] = row[w][c];
            }
        }
    }
}

int ClassificationInference::postprocess_binary(
    const std::vector<std::vector<float>>& output_data,
    float& prob0,
    float& prob1
) {
    if (output_data.empty() || output_data[0].size() != 2) {
        throw std::runtime_error("Invalid output for binary classification");
    }

    auto probs = softmax(output_data[0]);
    prob0 = probs[0];
    prob1 = probs[1];
    return (prob1 > prob0) ? 1 : 0;
}

std::vector<float> ClassificationInference::softmax(const std::vector<float>& logits) {
    std::vector<float> probs(logits.size());
    float max_logit = *std::max_element(logits.begin(), logits.end());

    float sum = 0.0f;
    for (size_t i = 0; i < logits.size(); ++i) {
        probs[i] = std::exp(logits[i] - max_logit);
        sum += probs[i];
    }
    for (float& v : probs) {
        v /= sum;
    }
    return probs;
}
}  // namespace vla
}  // namespace qrb