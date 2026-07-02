/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "vla_state_manager.hpp"

namespace qrb
{
namespace vla
{
VLAStateManager::VLAStateManager()
{
  printf("[%s]: VLAStateManager\n", logger_);
  init();
}

VLAStateManager::~VLAStateManager() {}

void VLAStateManager::register_publish_vla_state_callback(publish_vla_state_func_t cb)
{
  terminator_->register_publish_vla_state_callback(cb);
}

void VLAStateManager::update_image(cv::Mat &img)
{
  inference_->inference(img);
}

void VLAStateManager::update_image(cv::Mat &img, int64_t timestamp)
{
  inference_->inference(img);
}

void VLAStateManager::update_image_and_check(cv::Mat &img, uint32_t dir_idx)
{
  inference_->inference(img, dir_idx);
}

bool VLAStateManager::get_current_claw_state()
{
  return terminator_->get_current_claw_state();
}

void VLAStateManager::init()
{
  printf("[%s]: init\n", logger_);
  terminator_ = std::shared_ptr<Terminator>(new Terminator());
  inference_ = std::shared_ptr<ClassificationInference>(new ClassificationInference(terminator_));
  
}
}  // namespace vla
}  // namespace qrb
