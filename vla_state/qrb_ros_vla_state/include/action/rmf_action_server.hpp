/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS_VLA__RMF_ACTION_SERVER_HPP_
#define QRB_ROS_VLA__RMF_ACTION_SERVER_HPP_

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "qrb_ros_replenish_msgs/action/termination.hpp"
#include "manager/vla_state_manager.hpp"

using namespace qrb::vla;

using Termination = qrb_ros_replenish_msgs::action::Termination;
using GoalHandleTermination = rclcpp_action::ServerGoalHandle<Termination>;

namespace qrb_ros
{
namespace vla
{
class RMFActionServer : public rclcpp::Node
{
private:
  rclcpp_action::Server<Termination>::SharedPtr server_ptr_;
  std::shared_ptr<GoalHandleTermination> server_global_handle_;
  std::shared_ptr<VLAStateManager> manager_;
  publish_vla_state_func_t publish_vla_state_cb_;
  bool start_;
  uint8_t current_working_count = 0;
  uint8_t current_idle_count = 0;
  uint8_t last_working_count = 0;
  uint8_t last_idle_count = 0;

  void create_server();
  rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID & uuid,
      std::shared_ptr<const Termination::Goal> goal);
  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleTermination> goal_handle);
  void handle_accepted(const std::shared_ptr<GoalHandleTermination> goal_handle);
  void send_vla_state(bool working);

  rclcpp::Logger logger_{ rclcpp::get_logger("rmf_action_server") };

public:
  RMFActionServer(std::shared_ptr<VLAStateManager> &manager, const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~RMFActionServer();
};
}  // namespace vla
}  // namespace qrb_ros
#endif  // QRB_ROS_VLA__RMF_ACTION_SERVER_HPP_
