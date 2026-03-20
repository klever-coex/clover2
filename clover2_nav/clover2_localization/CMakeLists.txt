cmake_minimum_required(VERSION 3.8)
project(clover2_localization)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(clover2_common REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(g2o REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(image_geometry REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core aruco)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

set(dependencies
  clover2_common
  cv_bridge
  geometry_msgs
  lifecycle_msgs
  image_geometry
  pluginlib
  rclcpp
  rclcpp_lifecycle
  sensor_msgs
  tf2_eigen
  tf2_geometry_msgs
)

file(GLOB_RECURSE library_sources
  "src/graph/*.cpp"
  "src/handler/*.cpp"
  "src/queue/*.cpp"
)

file(GLOB sensor_sources "src/sensor/*.cpp")

clover2_add_library(
  TARGET clover2_localization
  SOURCES ${library_sources}
  LINK_LIBS g2o::core opencv_aruco
  DEPENDENCIES ${dependencies}
)

add_library(clover2_localization_sensors SHARED ${sensor_sources})
target_include_directories(clover2_localization_sensors PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(clover2_localization_sensors clover2_localization)
ament_target_dependencies(clover2_localization_sensors
  cv_bridge pluginlib rclcpp sensor_msgs)
ament_export_libraries(clover2_localization_sensors)

pluginlib_export_plugin_description_file(clover2_localization
  plugins/sensors_plugins.xml)

clover2_add_library(
  TARGET clover2_localization_node
  SOURCES src/node/localization.cpp
  LINK_LIBS clover2_localization
  DEPENDENCIES ${dependencies}
)

clover2_add_executable(
  TARGET localization
  SOURCES src/main.cpp
  LINK_LIBS clover2_localization_node
  DEPENDENCIES ${dependencies}
)

install(TARGETS clover2_localization clover2_localization_sensors
  clover2_localization_node localization
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

ament_export_include_directories(include)
ament_export_dependencies(${dependencies})
ament_export_libraries(clover2_localization clover2_localization_node
  clover2_localization_sensors)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
