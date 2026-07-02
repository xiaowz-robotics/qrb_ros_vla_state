/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "image_subscriber.hpp"
#include "task/task_manager.hpp"

ImageSubscriber::ImageSubscriber(std::shared_ptr<qrb_ros::vla::Task_Manager> & manager,
    const rclcpp::NodeOptions & options)
  : Node("image_sub", options)
  , manager_(manager)
{
  init_subscription();
}

ImageSubscriber::~ImageSubscriber() {}

void ImageSubscriber::init_subscription()
{
  //RCLCPP_INFO(logger_, "init_subscription");
  using namespace std::placeholders;
  image_sub0_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/cam0_stream1_rgb", 30, std::bind(&ImageSubscriber::image_sub_callback0, this, std::placeholders::_1));
}

void ImageSubscriber::image_sub_callback0(sensor_msgs::msg::Image::SharedPtr image_msg)
{
  RCLCPP_INFO(logger_, "-------------------image_sub_callback0");
  cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(image_msg, sensor_msgs::image_encodings::BGR8);
  //manager_->update_image(cv_ptr->image);
}
