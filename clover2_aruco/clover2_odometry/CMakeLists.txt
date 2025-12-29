cmake_minimum_required(VERSION 3.8)
project(clover2_odometry)

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
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc features2d video)

# Define common dependencies list for reuse
set(dependencies
  clover2_common
  cv_bridge
  diagnostic_updater
  geometry_msgs
  image_geometry
  nav_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  sensor_msgs
  tf2_geometry_msgs
  tf2_ros
)

# ============================================================================
# Build mono_visual_odometry component as a shared library
# ============================================================================
add_library(clover2_odometry_mono_visual_odometry src/mono_visual_odometry.cpp)
# Create alias for cleaner usage
add_library(clover2_odometry::mono_visual_odometry ALIAS clover2_odometry_mono_visual_odometry)

# Set C/C++ language standards
target_compile_features(clover2_odometry_mono_visual_odometry PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_odometry_mono_visual_odometry PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against OpenCV library
target_link_libraries(clover2_odometry_mono_visual_odometry
  ${OpenCV_LIBS}
)

# Define compilation flag for the library
target_compile_definitions(clover2_odometry_mono_visual_odometry PRIVATE "CLOVER2_ODOMETRY_MONO_VISUAL_ODOMETRY_BUILDING_LIBRARY")

# Add ROS 2 dependencies
ament_target_dependencies(clover2_odometry_mono_visual_odometry
  ${dependencies}
)

# Register as a ROS 2 component node
rclcpp_components_register_nodes(clover2_odometry_mono_visual_odometry "clover2_odometry::mono_visual_odometry")

# ============================================================================
# Build mono_visual_odometry executable (main application)
# ============================================================================
add_executable(mono_visual_odometry src/main.cpp)

# Set include directories
target_include_directories(mono_visual_odometry PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against library and OpenCV
target_link_libraries(mono_visual_odometry
  clover2_odometry_mono_visual_odometry
  ${OpenCV_LIBS}
)

ament_target_dependencies(mono_visual_odometry
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
  TARGETS mono_visual_odometry
  DESTINATION lib/${PROJECT_NAME}
)

# Install libraries and create export set for other packages to use
install(
  TARGETS clover2_odometry_mono_visual_odometry
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
  clover2_odometry_mono_visual_odometry
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
