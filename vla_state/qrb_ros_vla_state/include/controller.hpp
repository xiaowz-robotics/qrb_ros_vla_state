/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS_VLA__VLA_CONTROLLER_HPP_
#define QRB_ROS_VLA__VLA_CONTROLLER_HPP_

#include "image_subscriber.hpp"
#include "action/rmf_action_server.hpp"
#include "task/task_manager.hpp"
#include "manager/vla_state_manager.hpp"
#include "ros_common.hpp"
#include "tester.hpp"

using namespace qrb::vla;

namespace qrb_ros
{
namespace vla
{

class Controller
{
private:
  std::shared_ptr<VLAStateManager> vla_state_manager_;
  std::shared_ptr<ImageSubscriber> image_sub_;
  std::shared_ptr<RMFActionServer> rmf_server_;
  std::shared_ptr<Task_Manager> manager_;
  std::shared_ptr<Tester> tester_;

  void init_nodes();

  rclcpp::Logger logger_{ rclcpp::get_logger("vla_controller") };

public:
  Controller();
  ~Controller();

  std::shared_ptr<rclcpp::executors::MultiThreadedExecutor> executor_;
};
}  // namespace vla
}  // namespace qrb_ros
#endif  // QRB_ROS_VLA__VLA_CONTROLLER_HPP_
