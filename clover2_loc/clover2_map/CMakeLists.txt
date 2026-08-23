cmake_minimum_required(VERSION 3.20)

project(clover2_map)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_data SHARED DIRECTORY "src/data")

ament_auto_add_library(${PROJECT_NAME}_io SHARED DIRECTORY "src/io")
target_link_libraries(${PROJECT_NAME}_io ${PROJECT_NAME}_data yaml-cpp)

ament_auto_add_library(${PROJECT_NAME}_server SHARED "src/server.cpp")
target_link_libraries(${PROJECT_NAME}_server ${PROJECT_NAME}_io ${PROJECT_NAME}_data)

rclcpp_components_register_node(${PROJECT_NAME}_server
  PLUGIN "clover2_map::server"
  EXECUTABLE server
)

install(
  DIRECTORY map
  DESTINATION share/${PROJECT_NAME}
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
