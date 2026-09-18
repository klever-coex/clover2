cmake_minimum_required(VERSION 3.8)
project(clover2_http_plugins)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(plugins SHARED DIRECTORY "src")
target_link_libraries(plugins ${clover2_http_LIBRARIES})

pluginlib_export_plugin_description_file(clover2_http plugins.xml)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
