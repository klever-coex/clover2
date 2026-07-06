cmake_minimum_required(VERSION 3.8)
project(clover2_led)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(rosidl_default_generators REQUIRED)

ament_auto_find_build_dependencies()

file(GLOB msg_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "msg/*.msg")

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME}
    "rosidl_typesupport_cpp")

ament_auto_add_library(${PROJECT_NAME}_driver
  src/led_driver.cpp
)

target_link_libraries(${PROJECT_NAME}_driver ${cpp_typesupport_target})

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
