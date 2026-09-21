cmake_minimum_required(VERSION 3.8)

project(clover2_aruco)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(clover2_aruco_tracker SHARED DIRECTORY src)
target_compile_definitions(clover2_aruco_tracker PRIVATE
  CLOVER2_ARUCO_TRACKER_NODE_BUILDING_LIBRARY
)
rclcpp_components_register_node(clover2_aruco_tracker
  PLUGIN "clover2::aruco::tracker"
  EXECUTABLE tracker
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
