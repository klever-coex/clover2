cmake_minimum_required(VERSION 3.20)
project(clover2_http)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(Boost REQUIRED COMPONENTS url)
find_package(rosidl_default_generators REQUIRED)

ament_auto_find_build_dependencies()

file(GLOB plugin_msg_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "msg/*.msg")
rosidl_generate_interfaces(${PROJECT_NAME} ${plugin_msg_files})

ament_auto_add_library(${PROJECT_NAME}_http SHARED DIRECTORY "src/http")
target_link_libraries(${PROJECT_NAME}_http Boost::url)

ament_auto_add_library(${PROJECT_NAME}_plugins SHARED DIRECTORY "src/plugins")
ament_export_libraries(${PROJECT_NAME}_http ${PROJECT_NAME}_plugins)

ament_auto_add_library(${PROJECT_NAME}_node SHARED src/server.cpp)
rclcpp_components_register_node(${PROJECT_NAME}_node
  PLUGIN "clover2_http::server"
  EXECUTABLE server
)
ament_auto_add_executable(example examples/example.cpp)

pluginlib_export_plugin_description_file(clover2_http plugins.xml)

install(
  DIRECTORY launch params
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  file(GLOB test_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/*.cpp")
  ament_auto_add_gtest(test_${PROJECT_NAME}
    ${test_source}
  )

  file(GLOB plugin_test_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/plugins/*.cpp")
  ament_auto_add_gtest(test_${PROJECT_NAME}_plugins
    ${plugin_test_source}
  )
  target_link_libraries(test_${PROJECT_NAME}_plugins
    ${PROJECT_NAME}__rosidl_typesupport_cpp
    ${PROJECT_NAME}__rosidl_typesupport_introspection_cpp
    ${PROJECT_NAME}__rosidl_typesupport_fastrtps_cpp
  )
  target_compile_definitions(test_${PROJECT_NAME}_plugins PRIVATE
    AMENT_PREFIX="${CMAKE_INSTALL_PREFIX}"
  )
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
