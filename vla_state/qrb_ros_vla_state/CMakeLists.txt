cmake_minimum_required(VERSION 3.8)
project(qrb_ros_vla_state VERSION 1.0.3)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rcutils REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(qrb_vla_state_manager REQUIRED)
find_package(qrb_ros_replenish_msgs REQUIRED)
ament_auto_find_build_dependencies()

include_directories(${qrb_vla_state_manager_INCLUDE_DIRS})

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/action
    ${PROJECT_SOURCE_DIR}/include/task
    ${PROJECT_SOURCE_DIR}/include/queue
    ${PROJECT_SOURCE_DIR}/include/topic
    ${PROJECT_SOURCE_DIR}/include/test
    ${PROJECT_SOURCE_DIR}/include/vla_state_manager
    ${OpenCV_INCLUDE_DIRS}
    )

set(source
  src/main.cpp
  src/controller.cpp
  src/action/rmf_action_server.cpp
  src/task/task_manager.cpp
  src/topic/image_subscriber.cpp
  src/topic/vla_state_publisher.cpp
  src/test/tester.cpp
)

ament_auto_add_executable(qrb_ros_vla_state ${source})

ament_target_dependencies(qrb_ros_vla_state
  "geometry_msgs"
  "rclcpp"
  "rclcpp_action"
  "std_msgs"
  "rcutils"
  "sensor_msgs"
  "cv_bridge"
  "OpenCV"
  "qrb_vla_state_manager"
  "qrb_ros_replenish_msgs"
)

install(TARGETS qrb_ros_vla_state
    RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_auto_package(INSTALL_TO_SHARE launch)
