/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_VLA__MESSAGE_QUEUE_HPP_
#define QRB_VLA__MESSAGE_QUEUE_HPP_

#include "message.hpp"
#include <queue>

namespace qrb
{
namespace vla
{

class MessageQueue
{
public:
  void push(const Message & msg)
  {
    std::unique_lock<std::mutex> lck(mtx_);
    queue_.push(msg);
    cv_.notify_one();
  }

  void wait(Message & msg)
  {
    std::unique_lock<std::mutex> lck(mtx_);
    while (!queue_.size())
      cv_.wait(lck);
      
    msg = queue_.front();
    queue_.pop();
  }

  size_t size()
  {
    std::unique_lock<std::mutex> lck(mtx_);
    return queue_.size();
  }

  void notify()
  {
    std::unique_lock<std::mutex> lck(mtx_);
    Message msg;
    msg.timestamp = 0;
    queue_.push(msg);
    cv_.notify_one();
  }

private:
  std::queue<Message> queue_;
  std::mutex mtx_;
  std::condition_variable cv_;
};
}  // namespace vla
}  // namespace qrb
#endif  // QRB_VLA__MESSAGE_QUEUE_HPP_