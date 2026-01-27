cmake_minimum_required(VERSION 3.8)

project(clover2_aruco)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

# Find ROS 2 message and dependency packages
find_package(clover2_aruco_msgs REQUIRED)
find_package(clover2_common REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(g2o REQUIRED)
find_package(image_geometry REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(yaml-cpp REQUIRED)

# Find OpenCV with ArUco module support
find_package(OpenCV ${OpenCV_VERSION} REQUIRED COMPONENTS aruco)

# Define common dependencies list for reuse
set(dependencies
  clover2_aruco_msgs
  clover2_common
  cv_bridge
  diagnostic_updater
  geometry_msgs
  image_geometry
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  sensor_msgs
  std_msgs
  tf2_eigen
  tf2_geometry_msgs
  tf2_ros
)

# Common helper macros from clover2_common
# The extras file is loaded by find_package(clover2_common).

# ============================================================================
# Build ArUco utils library
# ============================================================================
file(GLOB_RECURSE utils_library_files "src/util/*.cpp")
clover2_add_library(
  TARGET clover2_aruco_utils
  SOURCES ${utils_library_files}
  LINK_LIBS yaml-cpp
  DEPENDENCIES ${dependencies}
)

# ============================================================================
# Build ArUco detector component as a shared library
# ============================================================================
clover2_add_component_library(
  TARGET clover2_aruco_detector
  SOURCES src/detector/detector.cpp
  DEFINES CLOVER2_ARUCO_DETECTOR_BUILDING_LIBRARY
  LINK_LIBS opencv_aruco
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::detector"
)

# ============================================================================
# Build ArUco map server component as a shared library
# ============================================================================
clover2_add_component_library(
  TARGET clover2_aruco_map_server
  SOURCES src/map_server/map_server.cpp
  DEFINES CLOVER2_ARUCO_MAP_SERVER_BUILDING_LIBRARY
  LINK_LIBS clover2_aruco_utils opencv_aruco
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::map_server"
)

# ============================================================================
# Build ArUco tracker component as a shared library
# ============================================================================

file(GLOB_RECURSE tracker_library_files "src/tracker/*.cpp")
clover2_add_component_library(
  TARGET clover2_aruco_tracker
  SOURCES ${tracker_library_files}
  DEFINES CLOVER2_ARUCO_TRACKER_BUILDING_LIBRARY
  LINK_LIBS g2o::core
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::tracker"
)

# ============================================================================
# Build detector executable (main application)
# ============================================================================
clover2_add_executable(
  TARGET detector
  SOURCES src/detector.cpp
  LINK_LIBS clover2_aruco_detector opencv_aruco
  DEPENDENCIES ${dependencies}
)

# ============================================================================
# Build map_server executable (main application)
# ============================================================================
clover2_add_executable(
  TARGET map_server
  SOURCES src/map_server.cpp
  LINK_LIBS clover2_aruco_map_server opencv_aruco
  DEPENDENCIES ${dependencies}
)

# ============================================================================
# Build tracker executable (main application)
# ============================================================================
clover2_add_executable(
  TARGET tracker
  SOURCES src/tracker.cpp
  LINK_LIBS clover2_aruco_tracker opencv_aruco
  DEPENDENCIES ${dependencies}
)

# ============================================================================
# Installation configuration
# ============================================================================

# Install map directory to share location
install(
  DIRECTORY map
  DESTINATION share/${PROJECT_NAME}
)

# Install header files
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install executables
install(
  TARGETS map_server detector tracker
  DESTINATION lib/${PROJECT_NAME}
)

# Install libraries and create export set for other packages to use
install(
  TARGETS clover2_aruco_utils clover2_aruco_map_server clover2_aruco_detector clover2_aruco_tracker
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
  clover2_aruco_map_server
  clover2_aruco_detector
  clover2_aruco_tracker
)

# Export CMake targets
ament_export_targets(
  export_${PROJECT_NAME}
)

# ============================================================================
# Testing configuration (only if BUILD_TESTING is enabled)
# ============================================================================
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
