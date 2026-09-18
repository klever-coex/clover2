cmake_minimum_required(VERSION 3.8)
project(clover2_mavros)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(mavlink REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(clover2_mavros_plugins SHARED src/plugins/clover2_vio.cpp)
target_include_directories(clover2_mavros_plugins PUBLIC ${mavlink_INCLUDE_DIRS})
target_link_libraries(clover2_mavros_plugins Eigen3::Eigen ${mavros_LIBRARIES})

pluginlib_export_plugin_description_file(mavros mavros_plugins.xml)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
