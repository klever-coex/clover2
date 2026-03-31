cmake_minimum_required(VERSION 3.8)

project(clover2_map_server)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(clover2_aruco_msgs REQUIRED)
find_package(clover2_common REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(yaml-cpp REQUIRED)

set(dependencies
  clover2_aruco_msgs
  clover2_common
  geometry_msgs
  rclcpp
  rclcpp_components
  rclcpp_lifecycle
  std_msgs
  tf2_eigen
  tf2_geometry_msgs
  tf2_ros
)

clover2_add_library(
  TARGET clover2_map_server_io
  SOURCES src/util/map_io.cpp
  LINK_LIBS yaml-cpp
  DEPENDENCIES ${dependencies}
)

clover2_add_component_library(
  TARGET clover2_map_server_node
  SOURCES src/map_server/map_server.cpp
  DEFINES CLOVER2_MAP_SERVER_NODE_BUILDING_LIBRARY
  LINK_LIBS clover2_map_server_io
  DEPENDENCIES ${dependencies}
  COMPONENT_CLASS "clover2::map_server::map_server"
)

clover2_add_executable(
  TARGET map_server
  SOURCES src/main.cpp
  LINK_LIBS clover2_map_server_node
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
  TARGETS clover2_map_server_io clover2_map_server_node
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_dependencies(${dependencies})
ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_libraries(clover2_map_server_node clover2_map_server_io)
ament_export_targets(export_${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
