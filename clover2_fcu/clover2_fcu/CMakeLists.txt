cmake_minimum_required(VERSION 3.8)
project(clover2_fcu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME} SHARED DIRECTORY src)
target_link_libraries(${PROJECT_NAME} yaml-cpp Eigen3::Eigen)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "clover2_fcu::bridge"
  EXECUTABLE bridge
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_auto_add_gtest(test_threading test/test_threading.cpp)
  ament_auto_add_gtest(test_command_handle test/test_command_handle.cpp)

  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
