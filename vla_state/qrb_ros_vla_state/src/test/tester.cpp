/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "tester.hpp"
#include <iostream>
#include <random>

namespace qrb_ros
{
namespace vla
{
Tester::Tester(std::shared_ptr<VLAStateManager> &manager,
std::shared_ptr<qrb_ros::vla::Task_Manager> & task_manager)
                         : manager_(manager), task_manager_(task_manager)
{
  current_dir_idx_ = 0;
  thread_ = std::make_shared<std::thread>(std::mem_fn(&Tester::handle_test), this);
}

Tester::~Tester()
{
  if (thread_->joinable()) {
    thread_->join();
  }
}

void Tester::handle_test()
{
  //std::string path1 = "/home/ubuntu/vla/test_image/0/";
  std::string path1 = "/home/ubuntu/vla/test_image/1/";
  dirs_.push_back(path1);

  std::cout << "current dir: " << dirs_[current_dir_idx_] << std::endl;

  std::vector<cv::String> file_names;
  std::string pattern = dirs_[current_dir_idx_] + "*.jpg";
  cv::glob(pattern, file_names, false);

  if (file_names.empty()) {
    std::cout << "warning: find jpg failed: " << dirs_[current_dir_idx_] << std::endl;
    return;
  }

  // 按文件名排序
  std::sort(file_names.begin(), file_names.end());

  for (const auto & selected_path : file_names) {
    cv::Mat img = cv::imread(selected_path);
    if (!img.empty()) {
      std::cout << "---------------------------------------------------------------------[read] dir: " << (current_dir_idx_ + 1)
                << " | file: " << selected_path
                << " | size: " << img.cols << "x" << img.rows << std::endl;
      RCLCPP_INFO(logger_, "selected_path: %s", selected_path.c_str());
      manager_->update_image_and_check(img, current_dir_idx_);
      //task_manager_->update_image(img);
    } else {
      std::cerr << "[error] : " << selected_path << std::endl;
    }
  }
}
}  // namespace vla
}  // namespace qrb_ros