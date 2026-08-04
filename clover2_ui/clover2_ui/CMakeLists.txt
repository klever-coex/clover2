cmake_minimum_required(VERSION 3.8)
project(clover2_ui)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(yaml-cpp REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_api SHARED DIRECTORY "src/api")
target_link_libraries(${PROJECT_NAME}_api yaml-cpp)

ament_auto_add_library(${PROJECT_NAME}_tui SHARED DIRECTORY "src/tui")

ament_auto_add_executable(settings src/commands/settings.cpp)
ament_auto_add_executable(diagnostics
  src/commands/diagnostics.cpp
  src/commands/diagnostics_runtime.cpp
)

install(DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
