/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QRB_VLA_STATE_MANAGER__VLA_STATE_MANAGER_HPP_
#define QRB_VLA_STATE_MANAGER__VLA_STATE_MANAGER_HPP_

#include "qnn_inference/classification_inference.hpp"

typedef std::function<void(bool val)> publish_vla_state_func_t;

namespace qrb
{
namespace vla
{
/**
 * @class navigation_controller::VLAStateManager
 * @desc The VLAStateManager create nodes to control the Navigation.
 */
class VLAStateManager
{
public:
  /**
   * @desc A constructor for VLAStateManager
   */
  VLAStateManager();

  /**
   * @desc A destructor for VLAStateManager
   */
  ~VLAStateManager();

  void update_image(cv::Mat &img);
  void update_image(cv::Mat &img, int64_t timestamp);
  void update_image_and_check(cv::Mat &img, uint32_t dir_idx);
  bool get_current_claw_state();
  void register_publish_vla_state_callback(publish_vla_state_func_t cb);

private:
  std::shared_ptr<ClassificationInference> inference_;
  std::shared_ptr<Terminator> terminator_;
  void init();

  const char * logger_ = "vla_state_manager";
};
}  // namespace vla
}  // namespace qrb
#endif  // QRB_VLA_STATE_MANAGER__VLA_STATE_MANAGER_HPP_