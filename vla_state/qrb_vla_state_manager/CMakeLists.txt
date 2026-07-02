cmake_minimum_required(VERSION 3.8)
project(qrb_vla_state_manager VERSION 1.0.3)

set(library_name qrb_vla_state_manager)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
find_package(OpenCV REQUIRED)
find_package(rclcpp REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/manager
    ${PROJECT_SOURCE_DIR}/include/qnn_inference
    ${PROJECT_SOURCE_DIR}/include/terminator
    ${OpenCV_INCLUDE_DIRS}
    )

add_library(${library_name} SHARED
  src/manager/vla_state_manager.cpp
  src/qnn_inference/classification_inference.cpp
  src/qnn_inference/data_util.cpp
  src/qnn_inference/io_tensor.cpp
  src/qnn_inference/model.cpp
  src/terminator/terminator.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
)

target_link_libraries(qrb_vla_state_manager
        dl
        ${OpenCV_LIBS}
)

ament_target_dependencies(qrb_vla_state_manager
  OpenCV
  "rclcpp"
)

install(TARGETS qrb_vla_state_manager
  EXPORT export_qrb_vla_state_manager
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)

ament_export_include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
)

ament_auto_package()