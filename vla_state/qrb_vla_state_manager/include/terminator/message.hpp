/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB__MESSAGE_HPP_
#define QRB__MESSAGE_HPP_

#include <string>
#include <bitset>
#include <cfloat>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <map>
#include <queue>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

namespace qrb
{
namespace vla
{
class Message
{
public:
  const static int WORKING = 1;
  const static int STOPPING = 2;

  long timestamp;
  int moving;

  static std::string state_to_string(int moving)
  {
    std::string message;
    switch (moving) {
      case WORKING:
        message = "WORKING";
        break;
      case STOPPING:
        message = "STOPPING";
        break;
      default:
        message = "INVALID";
        break;
    }
    return message;
  }
};
}  // namespace vla
}  // namespace qrb
#endif  // QRB__MESSAGE_HPP_