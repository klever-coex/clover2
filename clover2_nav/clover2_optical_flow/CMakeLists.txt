cmake_minimum_required(VERSION 3.8)

project(clover2_optical_flow)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

# Find ROS 2 message and dependency packages
find_package(clover2_common REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(image_geometry REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Define common dependencies list for reuse
set(dependencies
  clover2_common
  cv_bridge
  diagnostic_updater
  geometry_msgs
  image_geometry
  mavros_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  sensor_msgs
  tf2_geometry_msgs
  tf2_ros
)

# ============================================================================
# Build optical_flow component as a shared library
# ============================================================================
add_library(clover2_optical_flow_component src/optical_flow.cpp)
# Create alias for cleaner usage
add_library(clover2::optical_flow::optical_flow ALIAS clover2_optical_flow_component)

# Set C/C++ language standards
target_compile_features(clover2_optical_flow_component PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_optical_flow_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against OpenCV library
target_link_libraries(clover2_optical_flow_component
  ${OpenCV_LIBS}
)

# Add ROS 2 dependencies
ament_target_dependencies(clover2_optical_flow_component
  ${dependencies}
)

# Register as a ROS 2 component node
rclcpp_components_register_nodes(clover2_optical_flow_component "clover2::optical_flow::optical_flow")

# ============================================================================
# Build optical_flow executable (main application)
# ============================================================================
add_executable(optical_flow src/main.cpp)

# Set include directories
target_include_directories(optical_flow PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against optical_flow component library
target_link_libraries(optical_flow
  clover2_optical_flow_component
)

ament_target_dependencies(optical_flow
  ${dependencies}
)

# ============================================================================
# Installation configuration
# ============================================================================

# Install header files
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install executables
install(
  TARGETS optical_flow
  DESTINATION lib/${PROJECT_NAME}
)

# Install libraries and create export set for other packages to use
install(
  TARGETS clover2_optical_flow_component
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
  clover2_optical_flow_component
)

# Export CMake targets
ament_export_targets(
  export_${PROJECT_NAME}
)

# ============================================================================
# Testing configuration (only if BUILD_TESTING is enabled)
# ============================================================================
if(BUILD_TESTING)
  # Find testing utilities
  find_package(ament_lint_auto REQUIRED)

  # Enable specific linters
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)

  # Automatically find and setup linting dependencies
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
