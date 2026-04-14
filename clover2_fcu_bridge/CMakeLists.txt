cmake_minimum_required(VERSION 3.8)

project(clover2_fcu_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

# Find ROS 2 message and dependency packages
find_package(clover2_common REQUIRED)
find_package(clover2_fcu_bridge_msgs REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Define common dependencies list for reuse
set(dependencies
  clover2_common
  clover2_fcu_bridge_msgs
  mavros_msgs
  sensor_msgs
  rclcpp
  rclcpp_action
  rclcpp_components
  rclcpp_lifecycle
  tf2_geometry_msgs
  tf2_ros
)

file(GLOB_RECURSE lib_source "src/lib/*.cpp")

clover2_add_component_library(
  TARGET clover2_fcu_bridge
  SOURCES ${lib_source}
  DEFINES CLOVER2_FCU_BRIDGE_SERVER_BUILDING_LIBRARY
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::fcu_bridge::server"
)

clover2_add_executable(
  TARGET fcu_bridge
  SOURCES src/main.cpp
  LINK_LIBS clover2_fcu_bridge
  DEPENDENCIES ${dependencies}
)

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS fcu_bridge
  DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS clover2_fcu_bridge
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
  clover2_fcu_bridge
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
