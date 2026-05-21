cmake_minimum_required(VERSION 3.8)
project(clover2_mavros)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(mavlink REQUIRED)
find_package(mavros REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(rcpputils REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(clover2_mavros_plugins SHARED src/plugins/clover2_vio.cpp)
target_include_directories(clover2_mavros_plugins PUBLIC ${mavlink_INCLUDE_DIRS})

ament_target_dependencies(
  clover2_mavros_plugins
  PUBLIC diagnostic_msgs
          diagnostic_updater
          geometry_msgs
          mavros
          mavros_msgs
          pluginlib
          rclcpp
          rcpputils
          sensor_msgs
          tf2_eigen
          tf2_ros)

target_link_libraries(clover2_mavros_plugins PUBLIC Eigen3::Eigen ${mavros_LIBRARIES})

pluginlib_export_plugin_description_file(mavros mavros_plugins.xml)

install(
  TARGETS clover2_mavros_plugins
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_package()
