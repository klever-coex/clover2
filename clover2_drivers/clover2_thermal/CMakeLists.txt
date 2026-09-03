cmake_minimum_required(VERSION 3.8)
project(clover2_thermal)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages.
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_camera SHARED src/camera.cpp)
target_include_directories(${PROJECT_NAME}_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
rclcpp_components_register_node(${PROJECT_NAME}_camera
  PLUGIN "clover2_thermal::camera"
  EXECUTABLE camera
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)