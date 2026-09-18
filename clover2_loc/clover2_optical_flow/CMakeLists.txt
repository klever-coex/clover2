cmake_minimum_required(VERSION 3.8)

project(clover2_optical_flow)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(OpenCV REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME} SHARED DIRECTORY src)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "clover2::optical_flow::optical_flow"
  EXECUTABLE optical_flow
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
