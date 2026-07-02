/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_ROS_ANKER__MESSAGE_QUEUE_HPP_
#define QRB_ROS_ANKER__MESSAGE_QUEUE_HPP_

#include "message.hpp"
#include <mutex>
#include <condition_variable>
#include <optional>

namespace qrb_ros
{
namespace vla
{

// Single-slot queue: only the latest message is retained.
class MessageQueue
{
public:
  // Overwrite the pending slot; old frame is discarded if not yet consumed.
  void push(const Message & msg)
  {
    {
      std::unique_lock<std::mutex> lck(mtx_);
      slot_ = msg;
    }
    cv_.notify_one();
  }

  // Block until a message is available or notify() wakes us; returns false on shutdown.
  bool wait(Message & msg)
  {
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this] { return slot_.has_value() || stopped_; });
    if (stopped_ && !slot_.has_value()) {
      return false;
    }
    msg = std::move(slot_.value());
    slot_.reset();
    return true;
  }

  // Wake up the waiting thread to allow graceful shutdown.
  void notify()
  {
    {
      std::unique_lock<std::mutex> lck(mtx_);
      stopped_ = true;
    }
    cv_.notify_one();
  }

  size_t size()
  {
    std::unique_lock<std::mutex> lck(mtx_);
    return slot_.has_value() ? 1 : 0;
  }

private:
  std::optional<Message> slot_;
  bool stopped_ = false;
  std::mutex mtx_;
  std::condition_variable cv_;
};
}  // namespace vla
}  // namespace qrb_ros
#endif  // QRB_ROS_ANKER__MESSAGE_QUEUE_HPP_