/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS__VLA_STATE_PUBLISHER_HPP_
#define QRB_ROS__VLA_STATE_PUBLISHER_HPP_

#include <string>
#include "std_msgs/msg/bool.hpp"
#include "rclcpp/rclcpp.hpp"
#include "ros_common.hpp"
#include "manager/vla_state_manager.hpp"

using namespace qrb::vla;

namespace qrb_ros
{
namespace vla
{
class VLAStatePublisher : public rclcpp::Node {
public:
  VLAStatePublisher(std::shared_ptr<VLAStateManager>& manager);

  ~VLAStatePublisher();

  void send_vla_state(bool end);

private:
  void init_publisher();
  
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pub_;
  publish_vla_state_func_t publish_vla_state_cb_;
  std::shared_ptr<VLAStateManager> manager_;
  rclcpp::Logger logger_{rclcpp::get_logger("vla_state_publisher")};
};

}  // namespace vla
}  // namespace qrb_ros
#endif // QRB_ROS__VLA_STATE_PUBLISHER_HPP_