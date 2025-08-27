cmake_minimum_required(VERSION 3.8)
project(clover2_aruco)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(clover2_aruco_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

find_package(OpenCV ${OpenCV_VERSION} REQUIRED COMPONENTS aruco)

set(dependencies
  clover2_aruco_msgs
  cv_bridge
  geometry_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  sensor_msgs
  std_msgs
  tf2_geometry_msgs
  tf2_ros
)

add_library(clover2_aruco_detector src/detector.cpp)
add_library(clover2_aruco::detector ALIAS clover2_aruco_detector)
target_compile_features(clover2_aruco_detector PUBLIC c_std_99 cxx_std_17)
target_include_directories(clover2_aruco_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_link_libraries(clover2_aruco_detector
  opencv_aruco
)

target_compile_definitions(clover2_aruco_detector PRIVATE "CLOVER2_ARUCO_DETECTOR_BUILDING_LIBRARY")
ament_target_dependencies(clover2_aruco_detector
  ${dependencies}
)

rclcpp_components_register_nodes(clover2_aruco_detector "clover2_aruco::detector")

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS clover2_aruco_detector
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

add_executable(detector src/detector_node.cpp)
target_include_directories(detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_link_libraries(detector
  clover2_aruco_detector
  opencv_aruco
)

install(TARGETS detector
  DESTINATION lib/${PROJECT_NAME})

ament_target_dependencies(detector 
  ${dependencies}
)

ament_export_dependencies(${dependencies})

ament_export_include_directories(
  "include/${PROJECT_NAME}"
)

ament_export_libraries(
  clover2_aruco_detector
)

ament_export_targets(
  export_${PROJECT_NAME}
)

ament_package()
