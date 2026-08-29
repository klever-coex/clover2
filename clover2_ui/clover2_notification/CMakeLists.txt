cmake_minimum_required(VERSION 3.8)
project(clover2_notification)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(notification_outputs SHARED
  src/outputs/led.cpp)

pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

ament_auto_add_library(notification_core SHARED
  src/controller.cpp
  src/provider/factory.cpp
  src/provider/diagnostics.cpp)
target_link_libraries(notification_core notification_outputs)

ament_auto_add_executable(notification src/main.cpp)
target_link_libraries(notification notification_core)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  # The repository uses clang-format; do not register the incompatible
  # ament_uncrustify check for this package.
  set(ament_cmake_uncrustify_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  ament_auto_add_gtest(test_output test/test_output.cpp)

  ament_auto_add_gtest(test_led_output test/test_led_output.cpp)
  target_link_libraries(test_led_output notification_outputs)
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
