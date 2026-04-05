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
find_package(mavros_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# Define common dependencies list for reuse
set(dependencies
  clover2_common
  clover2_offboard_msgs
  diagnostic_updater
  mavros_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  tf2_geometry_msgs
  tf2_ros
)

clover2_add_library(
  TARGET clover2_offboard
  SOURCES
    src/backend/base_backend.cpp
    src/backend/fabric.cpp
    src/backend/mavros.cpp
    src/data/mode.cpp
    src/helper.cpp
  DEPENDENCIES rclcpp clover2_common mavros_msgs rclcpp_lifecycle
               tf2_geometry_msgs tf2_ros
)

clover2_add_component_library(
  TARGET clover2_offboard_server_component
  SOURCES src/server.cpp
  LINK_LIBS clover2_offboard
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2_offboard::server"
)

clover2_add_executable(
  TARGET offboard_server
  SOURCES src/main.cpp
  LINK_LIBS clover2_offboard_server_component clover2_offboard
  DEPENDENCIES ${dependencies}
)


install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS offboard_server
  DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS clover2_offboard clover2_offboard_server_component
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
  clover2_offboard
  clover2_offboard_server_component
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
