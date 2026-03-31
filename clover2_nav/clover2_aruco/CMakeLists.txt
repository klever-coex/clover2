cmake_minimum_required(VERSION 3.8)

project(clover2_aruco)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(clover2_aruco_msgs REQUIRED)
find_package(clover2_common REQUIRED)
find_package(clover2_map_server REQUIRED)
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

find_package(OpenCV ${OpenCV_VERSION} REQUIRED COMPONENTS aruco)

set(dependencies
  clover2_aruco_msgs
  clover2_common
  clover2_map_server
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

clover2_add_component_library(
  TARGET clover2_aruco_detector
  SOURCES src/detector/detector.cpp
  DEFINES CLOVER2_ARUCO_DETECTOR_BUILDING_LIBRARY
  LINK_LIBS opencv_aruco
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::detector"
)

file(GLOB_RECURSE tracker_library_files "src/tracker/*.cpp")
clover2_add_component_library(
  TARGET clover2_aruco_tracker
  SOURCES ${tracker_library_files}
  DEFINES CLOVER2_ARUCO_TRACKER_BUILDING_LIBRARY
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::aruco::tracker"
)

clover2_add_executable(
  TARGET detector
  SOURCES src/detector.cpp
  LINK_LIBS clover2_aruco_detector opencv_aruco
  DEPENDENCIES ${dependencies}
)

clover2_add_executable(
  TARGET tracker
  SOURCES src/tracker.cpp
  LINK_LIBS clover2_aruco_tracker opencv_aruco
  DEPENDENCIES ${dependencies}
)

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS detector tracker
  DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS clover2_aruco_detector clover2_aruco_tracker
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_dependencies(${dependencies})

ament_export_include_directories(
  "include/${PROJECT_NAME}"
)

ament_export_libraries(
  clover2_aruco_detector
  clover2_aruco_tracker
)

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
