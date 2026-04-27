cmake_minimum_required(VERSION 3.8)
project(clover2_cam_feature)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(clover2_map REQUIRED)
find_package(clover2_common REQUIRED)
find_package(clover2_pose_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(image_geometry REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS aruco)

set(dependencies
  clover2_map
  clover2_common
  clover2_pose_msgs
  cv_bridge
  diagnostic_updater
  geometry_msgs
  image_geometry
  pluginlib
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  sensor_msgs
  tf2
)

clover2_add_library(
  TARGET cam_feature_base
  SOURCES src/base_plugin.cpp
  DEPENDENCIES clover2_common clover2_map
  LINK_LIBS ${OpenCV_LIBRARIES}
)

clover2_add_library(
  TARGET cam_feature_plugins
  SOURCES
    src/detail/maker_base.cpp
    src/plugins/aruco.cpp
  DEPENDENCIES ${dependencies}
  LINK_LIBS cam_feature_base ${OpenCV_LIBRARIES}
)

pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

clover2_add_component_library(
  TARGET clover2_cam_feature_component
  SOURCES
    src/cam_feature.cpp
  LINK_LIBS cam_feature_plugins cam_feature_base ${OpenCV_LIBRARIES}
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::cam_feature::cam_feature"
)

clover2_add_executable(
  TARGET cam_feature
  SOURCES src/main.cpp
  LINK_LIBS clover2_cam_feature_component opencv_aruco
  DEPENDENCIES ${dependencies}
)

install(
  TARGETS cam_feature_base cam_feature_plugins clover2_cam_feature_component
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  TARGETS cam_feature
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include
  DESTINATION include/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(
  cam_feature_base cam_feature_plugins clover2_cam_feature_component)
ament_export_targets(export_${PROJECT_NAME})
ament_export_dependencies(${dependencies})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
