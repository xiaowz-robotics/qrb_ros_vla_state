/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS_ANKER__MESSAGE_HPP_
#define QRB_ROS_ANKER__MESSAGE_HPP_

#include <opencv2/opencv.hpp>
namespace qrb_ros
{
namespace vla
{
class Message
{
public:
  cv::Mat data;
  uint32_t count;
};

}  // namespace vla
}  // namespace qrb_ros
#endif  // QRB_ROS_ANKER__MESSAGE_HPP_