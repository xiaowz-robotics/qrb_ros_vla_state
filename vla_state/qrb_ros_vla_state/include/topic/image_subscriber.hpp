/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS_ANKER__IMAGE_SUBSCRIBER_HPP_
#define QRB_ROS_ANKER__IMAGE_SUBSCRIBER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <optional>

namespace qrb_ros
{
namespace vla
{
class Task_Manager;
}
}

class ImageSubscriber : public rclcpp::Node
{
private:
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub0_;
  std::shared_ptr<qrb_ros::vla::Task_Manager> manager_;
  void init_subscription();

  rclcpp::Logger logger_{rclcpp::get_logger("image_subscriber")};

public:
  ImageSubscriber(std::shared_ptr<qrb_ros::vla::Task_Manager> & manager,
      const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~ImageSubscriber();
  void image_sub_callback0(sensor_msgs::msg::Image::SharedPtr image_msg);
};

#endif  // QRB_ROS_ANKER__IMAGE_SUBSCRIBER_HPP_
