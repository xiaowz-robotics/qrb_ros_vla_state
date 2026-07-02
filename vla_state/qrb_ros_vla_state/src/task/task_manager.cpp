/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "task/task_manager.hpp"
#include <iostream>
#include <random>

namespace qrb_ros
{
namespace vla
{
Task_Manager::Task_Manager(std::shared_ptr<VLAStateManager> &manager)
                         : manager_(manager)
{
  running_ = true;
  thread_handle_msg_ =
      std::make_shared<std::thread>(std::mem_fn(&Task_Manager::handle_msg), this);
}

Task_Manager::~Task_Manager()
{
  running_ = false;
  queue_.notify();
  if (thread_handle_msg_->joinable()) {
    thread_handle_msg_->join();
  }
}

void Task_Manager::update_image(const cv::Mat & image)
{
  Message msg;
  msg.data = image;
  count_++;
  msg.count = count_;
  queue_.push(msg);
}

void Task_Manager::handle_msg()
{
  Message msg;
  while (running_) {
    if (!queue_.wait(msg)) {
      break;  // queue was shut down
    }
    if (!handle_message(msg)) {
      handle_failed(msg);
    }
  }
}

bool Task_Manager::handle_message(Message & msg)
{
  RCLCPP_INFO(logger_, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~count: %u", msg.count);
  save_image(msg);

  manager_->update_image(msg.data);
  return true;
}

void Task_Manager::handle_failed(Message & msg)
{
  (void)msg;
}

void Task_Manager::save_image(Message & msg)
{
  cv::Mat vis = msg.data;
  const std::string output_path = "/home/ubuntu/failed_result.jpg";
  if (cv::imwrite(output_path, vis)) {
        std::cout << "Saved annotated result to " << output_path << std::endl;
    } else {
        std::cerr << "Failed to write output image: " << output_path << std::endl;
    }
}
}  // namespace vla
}  // namespace qrb_ros
