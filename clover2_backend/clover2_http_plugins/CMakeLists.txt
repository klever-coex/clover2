cmake_minimum_required(VERSION 3.20)
project(clover2_http_plugins)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(rosidl_default_generators REQUIRED)

ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/JsonTestInner.msg"
  "msg/JsonTest.msg"
)

ament_auto_add_library(plugins SHARED DIRECTORY "src")

# clover2_http headers pull boost::urls symbols (request_context).
find_package(Boost REQUIRED COMPONENTS url)
target_link_libraries(plugins Boost::url)

install(FILES plugins.xml DESTINATION share/${PROJECT_NAME})
pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  # Linking the typesupport libraries keeps the .so files loaded in the
  # test process, so the dynamic lookups in typesupport/message_type and
  # msg_json resolve them by soname.
  ament_auto_add_gtest(test_msg_json test/test_msg_json.cpp)
  target_link_libraries(test_msg_json
    ${PROJECT_NAME}__rosidl_typesupport_cpp
    ${PROJECT_NAME}__rosidl_typesupport_introspection_cpp
  )

  ament_auto_add_gtest(test_rate_limiter test/test_rate_limiter.cpp)

  ament_auto_add_gtest(test_message_type test/test_message_type.cpp)
  target_compile_definitions(test_message_type
    PRIVATE AMENT_PREFIX="${CMAKE_INSTALL_PREFIX}")
  target_link_libraries(test_message_type
    ${PROJECT_NAME}__rosidl_typesupport_cpp
    ${PROJECT_NAME}__rosidl_typesupport_introspection_cpp
    ${PROJECT_NAME}__rosidl_typesupport_fastrtps_cpp
  )
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
