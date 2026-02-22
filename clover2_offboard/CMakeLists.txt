cmake_minimum_required(VERSION 3.8)

project(clover2_offboard)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

# Find ROS 2 message and dependency packages
find_package(clover2_common REQUIRED)
find_package(clover2_offboard_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Define common dependencies list for reuse
set(dependencies
    clover2_common
    clover2_offboard_msgs
    diagnostic_updater
    geometry_msgs
    mavros_msgs
    rclcpp
    rclcpp_components
    rclcpp_lifecycle
    sensor_msgs
    std_srvs
    tf2_geometry_msgs
    tf2_ros
)

# Common helper macros from clover2_common
# The extras file is loaded by find_package(clover2_common).
# ============================================================================
# Build ArUco utils library
# ============================================================================
file(GLOB_RECURSE offboard_library_files "src/offboard/*.cpp")
# list(FILTER offboard_library_files EXCLUDE REGEX ".*offboard_node\\.cpp$")
clover2_add_library(
  TARGET clover2_offboard
  SOURCES ${offboard_library_files}
  DEPENDENCIES ${dependencies}
)


# ============================================================================
# Installation configuration
# ============================================================================

# Install map directory to share location
# install(
#   DIRECTORY map
#   DESTINATION share/${PROJECT_NAME}
# )

# Install header files
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# ============================================================================
# Build offboard_node executable
# ============================================================================
clover2_add_executable(
  TARGET offboard_node
  SOURCES src/offboard_node.cpp
  DEPENDENCIES ${dependencies}
  LINK_LIBS clover2_offboard
)

# Install executables
install(
  TARGETS offboard_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install libraries and create export set for other packages to use
install(
  TARGETS clover2_offboard
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib      # Static libraries
  LIBRARY DESTINATION lib      # Shared libraries
  RUNTIME DESTINATION bin      # Executables (Windows)
)

# ============================================================================
# Export configuration for other packages
# ============================================================================

# Export package dependencies
ament_export_dependencies(${dependencies})

# Export include directories
ament_export_include_directories(
  "include/${PROJECT_NAME}"
)

# Export libraries for other packages to link against
ament_export_libraries(
  clover2_offboard
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
