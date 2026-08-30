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

rclcpp_components_register_node(notification_core
  PLUGIN "clover2_notification::controller"
  EXECUTABLE notification
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_auto_add_gtest(test_clover2_notification
    test/test_output.cpp
    test/test_led_output.cpp
  )
  target_link_libraries(test_clover2_notification notification_outputs)
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
