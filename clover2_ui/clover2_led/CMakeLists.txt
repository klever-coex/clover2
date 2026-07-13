cmake_minimum_required(VERSION 3.8)
project(clover2_led)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS 2 build system packages
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_device SHARED DIRECTORY "src/device")
target_link_libraries(${PROJECT_NAME}_device ${cpp_typesupport_target})

ament_auto_add_library(led_plugins SHARED DIRECTORY "src/plugins")
target_link_libraries(led_plugins
  ${cpp_typesupport_target}
  ${PROJECT_NAME}_device
)

pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

ament_auto_add_library(${PROJECT_NAME}_driver src/driver.cpp)
target_link_libraries(${PROJECT_NAME}_driver
  ${cpp_typesupport_target}
  ${PROJECT_NAME}_device
)

ament_auto_add_executable(driver src/main.cpp)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  file(GLOB test_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/*.cpp")
  ament_auto_add_gtest(test_${PROJECT_NAME}
    ${test_source}
  )

  target_link_libraries(test_${PROJECT_NAME} ${cpp_typesupport_target})
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
