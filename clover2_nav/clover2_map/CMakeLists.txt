cmake_minimum_required(VERSION 3.8)

project(clover2_map)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(clover2_pose_msgs REQUIRED)
find_package(clover2_common REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(yaml-cpp REQUIRED)

set(dependencies
  clover2_pose_msgs
  clover2_common
  geometry_msgs
  rclcpp
  rclcpp_components
  std_msgs
  tf2_eigen
  tf2_geometry_msgs
  tf2_ros
)

file(GLOB_RECURSE io_sources "src/io/*.cpp")

clover2_add_library(
  TARGET clover2_map_server_io
  SOURCES ${io_sources}
  LINK_LIBS yaml-cpp
  DEPENDENCIES ${dependencies}
)

file(GLOB_RECURSE component_sources "src/map/*.cpp")

clover2_add_component_library(
  TARGET clover2_map_server
  SOURCES ${component_sources}
  DEFINES CLOVER2_MAP_SERVER_NODE_BUILDING_LIBRARY
  LINK_LIBS clover2_map_server_io
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::map::server"
)

clover2_add_executable(
  TARGET map_server
  SOURCES src/main.cpp
  LINK_LIBS clover2_map_server
  DEPENDENCIES ${dependencies}
)

install(
  DIRECTORY map
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS map_server
  DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS clover2_map_server_io clover2_map_server
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_libraries(clover2_map_server clover2_map_server_io)
ament_export_targets(export_${PROJECT_NAME})
ament_export_dependencies(${dependencies})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
