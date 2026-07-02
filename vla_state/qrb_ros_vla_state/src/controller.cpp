/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "controller.hpp"

namespace qrb_ros
{
namespace vla
{

Controller::Controller()
{
  init_nodes();
  RCLCPP_INFO(logger_, "Init vla node");
}

Controller::~Controller() {}

void Controller::init_nodes()
{
  executor_ = std::shared_ptr<rclcpp::executors::MultiThreadedExecutor>(
      new rclcpp::executors::MultiThreadedExecutor());

  vla_state_manager_ = std::shared_ptr<VLAStateManager>(new VLAStateManager());

  rmf_server_ = std::shared_ptr<RMFActionServer>(new RMFActionServer(vla_state_manager_));

  manager_ = std::shared_ptr<Task_Manager>(new Task_Manager(vla_state_manager_));

  image_sub_ = std::shared_ptr<ImageSubscriber>(new ImageSubscriber(manager_));

  tester_ = std::shared_ptr<Tester>(new Tester(vla_state_manager_,manager_));

  executor_->add_node(image_sub_);
  executor_->add_node(rmf_server_);
}
}  // namespace vla
}  // namespace qrb_ros
