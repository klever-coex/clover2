cmake_minimum_required(VERSION 3.8)

project(clover2_aruco)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

# Find ROS 2 message and dependency packages
find_package(clover2_common REQUIRED)
find_package(clover2_map REQUIRED)
find_package(clover2_pose_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Define common dependencies list for reuse
set(dependencies
  clover2_common
  clover2_map
  clover2_pose_msgs
  diagnostic_updater
  geometry_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  std_msgs
  tf2_eigen
  tf2_geometry_msgs
  tf2_ros
)

clover2_add_component_library(
  TARGET clover2_aruco_tracker
  SOURCES src/tracker/tracker.cpp
  DEFINES CLOVER2_ARUCO_TRACKER_NODE_BUILDING_LIBRARY
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::tracker"
)

clover2_add_executable(
  TARGET tracker
  SOURCES src/tracker/main.cpp
  LINK_LIBS clover2_aruco_tracker
  DEPENDENCIES ${dependencies}
)

# Install header files
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install executables
install(
  TARGETS tracker
  DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS clover2_aruco_tracker
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Export package dependencies
ament_export_dependencies(${dependencies})

# Export include directories
ament_export_include_directories(
  "include/${PROJECT_NAME}"
)

# Export libraries for other packages to link against
ament_export_libraries(
  clover2_aruco_tracker
)

# Export CMake targets
ament_export_targets(
  export_${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
