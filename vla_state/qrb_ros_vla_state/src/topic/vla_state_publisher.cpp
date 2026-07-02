/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "vla_state_publisher.hpp"

constexpr char const *vla_state_node_name = "vla_state_pub";

namespace qrb_ros
{
namespace vla
{
using namespace std::placeholders;
using namespace std::chrono_literals;

VLAStatePublisher::VLAStatePublisher(std::shared_ptr<VLAStateManager>& manager)
    : Node(vla_state_node_name), manager_(manager) {
  RCLCPP_INFO(logger_, "Creating");
  init_publisher();

  publish_vla_state_cb_ = [&](bool val) {
    send_vla_state(val);
  };
  manager_->register_publish_vla_state_callback(publish_vla_state_cb_);
}

VLAStatePublisher::~VLAStatePublisher() { RCLCPP_INFO(logger_, "Destroying"); }

void VLAStatePublisher::init_publisher()
{
  RCLCPP_INFO(logger_, "init_publisher");

  using namespace std::placeholders;
  pub_ = create_publisher<std_msgs::msg::Bool>("vla_state", 10);
}

void VLAStatePublisher::send_vla_state(bool end) {
  std_msgs::msg::Bool msg;
  msg.data = end;
  pub_->publish(msg);
  if (end) {
    RCLCPP_INFO(logger_, "Publish VLA state: working");
  } else {
    RCLCPP_INFO(logger_, "Publish VLA state: stopping");
  }
}
}  // namespace vla
}  // namespace qrb_ros