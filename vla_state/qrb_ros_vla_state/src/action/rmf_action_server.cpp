/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "action/rmf_action_server.hpp"

namespace qrb_ros
{
namespace vla
{
RMFActionServer::RMFActionServer(std::shared_ptr<VLAStateManager> &manager, const rclcpp::NodeOptions & options)
  : Node("vla_action_server", options), manager_(manager)
{
  RCLCPP_INFO(logger_, "create_server");
  create_server();

  publish_vla_state_cb_ = [&](bool val) {
    send_vla_state(val);
  };
  manager_->register_publish_vla_state_callback(publish_vla_state_cb_);
}

void RMFActionServer::create_server()
{
  using namespace std::placeholders;
  this->server_ptr_ = rclcpp_action::create_server<Termination>(this->get_node_base_interface(),
      this->get_node_clock_interface(), this->get_node_logging_interface(),
      this->get_node_waitables_interface(), "/replenish/termination",
      std::bind(&RMFActionServer::handle_goal, this, _1, _2),
      std::bind(&RMFActionServer::handle_cancel, this, _1),
      std::bind(&RMFActionServer::handle_accepted, this, _1));
}

RMFActionServer::~RMFActionServer()
{
}

rclcpp_action::GoalResponse RMFActionServer::handle_goal(const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const Termination::Goal> goal)
{
  (void)uuid;
  RCLCPP_INFO(logger_, "Received request to handle goal");
  if (goal->command == 1) {
    start_ = true;
    RCLCPP_INFO(logger_, "Start publish VLA state");
  } else {
    start_ = false;
    RCLCPP_INFO(logger_, "Stop publish VLA state");
  }

  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse RMFActionServer::handle_cancel(
    const std::shared_ptr<GoalHandleTermination> goal_handle)
{
  (void)goal_handle;
  return rclcpp_action::CancelResponse::ACCEPT;
}

void RMFActionServer::handle_accepted(const std::shared_ptr<GoalHandleTermination> goal_handle)
{
  RCLCPP_INFO(logger_, "handle_accepted");
  server_global_handle_ = goal_handle;
}

void RMFActionServer::send_vla_state(bool working)
{
  if (working) {
    RCLCPP_INFO(logger_, "VLA state: working");
    current_working_count++;
    last_working_count = current_working_count;
    current_idle_count = 0;
  } else {
    RCLCPP_INFO(logger_, "VLA state: stopping");
    current_idle_count++;
    last_idle_count = current_idle_count;
    current_working_count = 0;
  }

  RCLCPP_INFO(logger_, "current_idle=%d, current_working=%d, last_idle=%d, last_working=%d",
     current_idle_count, current_working_count, last_idle_count, last_working_count);

  if (current_idle_count >= 3 && last_working_count >= 10) {
    RCLCPP_INFO(logger_, "Finish one task");
    last_working_count = 0;
  } else {
    return;
  }

  // feedback
  auto feedback = std::make_shared<Termination::Feedback>();
  feedback->success = true;

  if (server_global_handle_ != nullptr && !(server_global_handle_->is_canceling())) {
    server_global_handle_->publish_feedback(feedback);
    RCLCPP_INFO(this->get_logger(), "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@feedback one task");
  }
}
}  // namespace vla
}  // namespace qrb_ros
