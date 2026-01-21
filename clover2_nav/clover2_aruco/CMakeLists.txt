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

# ============================================================================
# Build ArUco utils library
# ============================================================================
file(GLOB_RECURSE utils_library_files "src/utils/*.cpp")
add_library(clover2_aruco_utils ${utils_library_files})

# Set C/C++ language standards
target_compile_features(clover2_aruco_utils PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_aruco_utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_link_libraries(clover2_aruco_utils
  yaml-cpp
)

# Add ROS 2 dependencies
ament_target_dependencies(clover2_aruco_utils
  ${dependencies}
)

# ============================================================================
# Build ArUco detector component as a shared library
# ============================================================================
add_library(clover2_aruco_detector src/detector/detector.cpp)
# Create alias for cleaner usage
add_library(clover2_aruco::detector ALIAS clover2_aruco_detector)

# Set C/C++ language standards
target_compile_features(clover2_aruco_detector PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_aruco_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against OpenCV ArUco library
target_link_libraries(clover2_aruco_detector
  opencv_aruco
)

# Define compilation flag for the detector library
target_compile_definitions(clover2_aruco_detector PRIVATE "CLOVER2_ARUCO_DETECTOR_BUILDING_LIBRARY")

# Add ROS 2 dependencies
ament_target_dependencies(clover2_aruco_detector
  ${dependencies}
)

# Register as a ROS 2 component node
rclcpp_components_register_nodes(clover2_aruco_detector "clover2_aruco::detector")

# ============================================================================
# Build ArUco map server component as a shared library
# ============================================================================
add_library(clover2_aruco_map_server src/map_server/map_server.cpp)
# Create alias for cleaner usage
add_library(clover2_aruco::map_server ALIAS clover2_aruco_map_server)

# Set C/C++ language standards
target_compile_features(clover2_aruco_map_server PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_aruco_map_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against OpenCV ArUco and YAML libraries
target_link_libraries(clover2_aruco_map_server
  clover2_aruco_utils
  opencv_aruco
)

# Define compilation flag for the map server library
target_compile_definitions(clover2_aruco_map_server PRIVATE "CLOVER2_ARUCO_MAP_SERVER_BUILDING_LIBRARY")

# Add ROS 2 dependencies
ament_target_dependencies(clover2_aruco_map_server
  ${dependencies}
)

# Register as a ROS 2 component node
rclcpp_components_register_nodes(clover2_aruco_map_server "clover2_aruco::map_server")

# ============================================================================
# Build ArUco tracker component as a shared library
# ============================================================================
add_library(clover2_aruco_tracker src/tracker/tracker.cpp)
# Create alias for cleaner usage
add_library(clover2_aruco::tracker ALIAS clover2_aruco_tracker)

# Set C/C++ language standards
target_compile_features(clover2_aruco_tracker PUBLIC c_std_99 cxx_std_17)

# Set include directories for both build and install interfaces
target_include_directories(clover2_aruco_tracker PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Define compilation flag for the map server library
target_compile_definitions(clover2_aruco_tracker PRIVATE "CLOVER2_ARUCO_TRACKER_BUILDING_LIBRARY")

# Add ROS 2 dependencies
ament_target_dependencies(clover2_aruco_tracker
  ${dependencies}
)

# Register as a ROS 2 component node
rclcpp_components_register_nodes(clover2_aruco_tracker "clover2_aruco::tracker")

# ============================================================================
# Build detector executable (main application)
# ============================================================================
add_executable(detector src/detector/main.cpp)

# Set include directories
target_include_directories(detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against detector library and OpenCV
target_link_libraries(detector
  clover2_aruco_detector
  opencv_aruco
)

ament_target_dependencies(detector
  ${dependencies}
)

# ============================================================================
# Build map_server executable (main application)
# ============================================================================
add_executable(map_server src/map_server/main.cpp)

# Set include directories
target_include_directories(map_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against map server library and OpenCV
target_link_libraries(map_server
  clover2_aruco_map_server
  opencv_aruco
)

ament_target_dependencies(map_server
  ${dependencies}
)

# ============================================================================
# Build tracker executable (main application)
# ============================================================================
add_executable(tracker src/tracker/main.cpp)

# Set include directories
target_include_directories(tracker PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against map server library and OpenCV
target_link_libraries(tracker
  clover2_aruco_tracker
  opencv_aruco
)

ament_target_dependencies(tracker
  ${dependencies}
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
  # Find testing utilities
  find_package(ament_lint_auto REQUIRED)

  # Enable specific linters
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)

  # Add Google Test for detector unit tests
  ament_add_gtest(${PROJECT_NAME}_tutorial_test
    test/unittest_detector.cpp
  )
  # Link test against detector library
  target_link_libraries(${PROJECT_NAME}_tutorial_test
    clover2_aruco_detector
  )

  # Automatically find and setup linting dependencies
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
