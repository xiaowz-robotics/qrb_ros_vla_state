/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS__TASK_MANAGER_HPP_
#define QRB_ROS__TASK_MANAGER_HPP_

#include <mutex>
#include <condition_variable>
#include <optional>
#include <thread>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "rclcpp/rclcpp.hpp"
#include "manager/vla_state_manager.hpp"
#include "queue/message_queue.hpp"

using namespace qrb::vla;

namespace qrb_ros
{
namespace vla
{
class Task_Manager
{
private:
  MessageQueue queue_;
  bool running_ = false;
  uint32_t count_ = 0;
  std::shared_ptr<std::thread> thread_handle_msg_;
  std::shared_ptr<VLAStateManager> manager_;
  rclcpp::Logger logger_{rclcpp::get_logger("task_manager")};

  bool handle_message(Message & msg);
  void handle_msg();
  void handle_failed(Message & msg);

  void save_image(Message & msg);

public:
  Task_Manager(std::shared_ptr<VLAStateManager> &manager);
  ~Task_Manager();

  void update_image(const cv::Mat & image);
};
}  // namespace vla
}  // namespace qrb_ros
#endif // QRB_ROS__TASK_MANAGER_HPP_
