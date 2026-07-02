/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS__TESTER_HPP_
#define QRB_ROS__TESTER_HPP_

#include <math.h>
#include "manager/vla_state_manager.hpp"
#include "task_manager.hpp"
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>
#include "rclcpp/rclcpp.hpp"

using namespace qrb::vla;

namespace qrb_ros
{
namespace vla
{
class Tester
{
private:
  std::shared_ptr<VLAStateManager> manager_;
  std::shared_ptr<qrb_ros::vla::Task_Manager> task_manager_;
  std::shared_ptr<std::thread> thread_;
  uint32_t current_dir_idx_;
  std::vector<std::string> dirs_;
  rclcpp::Logger logger_{rclcpp::get_logger("tester")};

  void handle_test();

public:
  Tester(std::shared_ptr<VLAStateManager> &manager, std::shared_ptr<qrb_ros::vla::Task_Manager> & task_manager);
  ~Tester();
};
}  // namespace vla
}  // namespace qrb_ros
#endif // QRB_ROS__TESTER_HPP_