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

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_http SHARED DIRECTORY "src/http")

find_package(Boost REQUIRED COMPONENTS url)
target_link_libraries(${PROJECT_NAME}_http Boost::url)

ament_auto_add_executable(example examples/example.cpp)

ament_auto_add_library(${PROJECT_NAME}_node SHARED src/server/node.cpp)
rclcpp_components_register_node(${PROJECT_NAME}_node
  PLUGIN "clover2_http::server::node"
  EXECUTABLE server
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  file(GLOB test_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/*.cpp")
  ament_auto_add_gtest(test_${PROJECT_NAME}
    ${test_source}
  )
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
