/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "terminator.hpp"
#include <iostream>

namespace qrb
{
namespace vla
{
Terminator::Terminator()
{
  running_ = true;
  current_claw_state_ = false;
  thread_handle_msg_ =
      std::make_shared<std::thread>(std::mem_fn(&Terminator::handle_msg), this);
  state_history_.clear();
}

Terminator::~Terminator()
{
  running_ = false;
  queue_.notify();
  if (thread_handle_msg_->joinable()) {
    thread_handle_msg_->join();
  }
}

bool Terminator::get_current_claw_state()
{
  std::unique_lock<std::mutex> lk(state_mutex_);
  bool res = current_claw_state_;
  return res;
}

void Terminator::register_publish_vla_state_callback(publish_vla_state_func_t cb)
{
  publish_vla_state_cb_ = cb;
}

void Terminator::handle_msg()
{
  Message msg;
  bool execute = true;
  while (running_) {
    queue_.wait(msg);
    handle_message(msg);
  }
}

void Terminator::update_manipulation_state(long timestamp,  bool moving) {
  Message msg;
  msg.timestamp = timestamp;
  if (moving) {
    msg.moving = Message::WORKING;
  } else {
    msg.moving = Message::STOPPING;
  }
  queue_.push(msg);
}

void Terminator::handle_message(const Message & msg)
{
  std::cout << "Receive message: " << Message::state_to_string(msg.moving) << std::endl;
  //RCLCPP_INFO(logger_, "handle_message");
  if (new_manipulation_data(msg.timestamp)) {
    push_state(msg.timestamp, msg.moving);
  }
  
  if (is_working()) {
    //RCLCPP_INFO(logger_, "+++++++publish_vla_state_cb_:true");
    publish_vla_state_cb_(true);
  } else {
    //RCLCPP_INFO(logger_, "-------publish_vla_state_cb_:false");
    publish_vla_state_cb_(false);
  }
}

void Terminator::push_state(long timestamp, int moving)
{
  Message msg;
  msg.timestamp = timestamp;
  msg.moving = moving;
  state_history_.push_back(msg);

  //remove old data with duration
  uint32_t n = state_history_.size();
  for (uint32_t i = 0; i < n; i++) {
    long old_timestamp = state_history_[0].timestamp;
    long duration = timestamp - old_timestamp;
    long max = (long)MAX_HISTORY_DURATION * 1000 * 1000 * 1000;
    //long max = 60 * 1000 * 1000;
    if (duration > max) {
      state_history_.erase(state_history_.begin());
      std::cout << "remove old manipulation data: " << old_timestamp << ", now:" << timestamp << ", duration:" << duration << ", max:" << max << std::endl;
    }
  }
  print_state_history(state_history_);
}

bool Terminator::is_working()
{
  //step1: 分段处理，把vector转化为vector<vector>，working状态连续的作为一个segment,stopping状态连续的作为一个segment
  std::vector<std::vector<Message>> segments;
  std::vector<Message> working_segment;
  std::vector<Message> stopping_segment;
  for (const auto &msg : state_history_) {
    if (msg.moving == Message::WORKING) {
      working_segment.push_back(msg);
      if (!stopping_segment.empty()) {
        segments.push_back(stopping_segment);
        stopping_segment.clear();
      }
    } else if (msg.moving == Message::STOPPING) {
      stopping_segment.push_back(msg);
      if (!working_segment.empty()) {
        segments.push_back(working_segment);
        working_segment.clear();
      }
    }
  }
  if (!working_segment.empty()) {
    segments.push_back(working_segment);
  }
  if (!stopping_segment.empty()) {
    segments.push_back(stopping_segment);
  }

  print_state_history(segments);

  //step2: 计算每个segment的持续个数，并删除过短（无效，count<3）的segment
  std::vector<std::vector<Message>> filtered_segments;
  uint32_t segments_size = segments.size();
  for (uint32_t i = 0; i < segments_size; i++) {
    auto segment = segments[i];
    if (i == 0) {
      filtered_segments.push_back(segment);
    } else if (i == (segments_size - 1)) {
      filtered_segments.push_back(segment);
    } else {
      if (valid_segment(segment)) {
        filtered_segments.push_back(segment);
      }
    }
  }
  print_state_history(filtered_segments);

  //step3: 判断是否正在工作
  if (filtered_segments.empty()) {
    std::cout << "filtered_segments is empty" << std::endl;
    return false;
  }

  //Step4: 把filtered_segments转化为vector<Message>
  std::vector<Message> filtered_state_history;
  for (const auto &segment : filtered_segments) {
    for (const auto &msg : segment) {
      filtered_state_history.push_back(msg);
    }
  }
  print_state_history(filtered_state_history);
  //Step5: 倒序判断state_history_中每一个segment的长度，长度大于INVALID_SEGMENT_SIZE,并返回这个segment的状态
  uint32_t n = filtered_state_history.size();
  int current_state = filtered_state_history[n - 1].moving;
  std::cout << "step5: current_state:" << current_state << std::endl;
  uint32_t count = 0;
  long start = 0;
  long end = 0;
  long duration = 0;
  for (int i = n - 1; i >= 0; --i) {
    if (filtered_state_history[i].moving == current_state) {
      count++;
      if (start == 0) {
        start = filtered_state_history[i].timestamp;
      }
    } else {
      end = filtered_state_history[i].timestamp;
      duration = (start - end) / (1000);
      std::cout << "step5: start:" << start << ", end:" << end << ", duration:" << duration << std::endl;
      if (duration >= INVALID_SEGMENT_DURATION) {
        break;
      }
      count = 0;
      start = 0;
      end = 0;
      current_state = filtered_state_history[i].moving;
      std::cout << "step5: valid segment, current_state:" << current_state << std::endl;
    }
  }
  if (end == 0) {
    end = filtered_state_history[0].timestamp;
    duration = (start - end) / (1000);
  }

  std::unique_lock<std::mutex> lk(state_mutex_);
  std::cout << "count:" << count << ", duration:" << duration << ", current_state:" << current_state << std::endl;
  if (current_state == Message::WORKING) {
    std::cout << "current state is WORKING" << std::endl;
    current_claw_state_ = true;
    return true;
  } else if (current_state == Message::STOPPING) {
    std::cout << "current state is STOPPING" << std::endl;
    current_claw_state_ = false;
    return false;
  }
}

void Terminator::print_state_history(std::vector<Message> &list)
{
  std::string str;
  for (const auto &msg : list) {
    str += std::to_string(msg.moving);
  }
  //printf("state_history: %s\n", str.c_str());
}

void Terminator::print_state_history(std::vector<std::vector<Message>> &list)
{
  std::string str;
  for (const auto &segment : list) {
    for (const auto &msg : segment) {
      str += std::to_string(msg.moving);
    }
  }
  //printf("state_history2: %s\n", str.c_str());
}

bool Terminator::new_manipulation_data(long current)
{
  uint32_t n = state_history_.size();
  if (n == 0) {
    return true;
  }
  long latest_state_timestamp = state_history_[n - 1].timestamp;
  if (current < latest_state_timestamp) {
    return false;
  }
  return true;
}

bool Terminator::valid_segment(std::vector<Message> &segment)
{
  uint32_t n = segment.size();
  long start = segment[0].timestamp;
  long end = segment[n - 1].timestamp;
  long duration = end - start;
  if (duration < (long)INVALID_SEGMENT_DURATION * 1000 * 1000 * 1000) {
	//std::cout << "invalid_segment：" << std::endl;
    return false;
  }
  return true;
}
}  // namespace vla
}  // namespace qrb