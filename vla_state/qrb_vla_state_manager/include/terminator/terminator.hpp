/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB__TERMINATOR_HPP_
#define QRB__TERMINATOR_HPP_

#include <math.h>
#include <memory>
#include <cstdint>
#include <vector>
#include "message_queue.hpp"
#include "rclcpp/rclcpp.hpp"
#define MAX_HISTORY_SIZE 40
#define INVALID_SEGMENT_SIZE 3

#define MAX_HISTORY_DURATION 60 //s
#define INVALID_SEGMENT_DURATION 3  //s

typedef std::function<void(bool val)> publish_vla_state_func_t;

namespace qrb
{
namespace vla
{
class Terminator
{
private:
  MessageQueue queue_;
  std::shared_ptr<std::thread> thread_handle_msg_;
  uint32_t continuous_working_count_{0};
  bool last_state_{false};
  bool running_{false};
  std::vector<Message> state_history_;
  publish_vla_state_func_t publish_vla_state_cb_;
  bool current_claw_state_;
  std::mutex state_mutex_;
  rclcpp::Logger logger_{rclcpp::get_logger("terminator")};

  void handle_message(const Message & msg);
  void handle_msg();
  void push_state(long timestamp,  int moving);
  bool is_working();
  void print_state_history(std::vector<Message> &list);
  void print_state_history(std::vector<std::vector<Message>> &list);
  bool new_manipulation_data(long current);
  bool valid_segment(std::vector<Message> &segment);

public:
  Terminator();
  ~Terminator();
  void update_manipulation_state(long timestamp,  bool state);
  void register_publish_vla_state_callback(publish_vla_state_func_t cb);
  bool get_current_claw_state();
};
}  // namespace vla
}  // namespace qrb
#endif // QRB__TERMINATOR_HPP_