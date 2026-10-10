cmake_minimum_required(VERSION 3.8)

project(clover2_fcu_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(ament_cmake_python REQUIRED)

ament_auto_find_build_dependencies()

ament_python_install_package(${PROJECT_NAME}
  PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/${PROJECT_NAME}"
)

ament_auto_add_library(clover2_fcu_bridge SHARED DIRECTORY "src/lib")
target_compile_definitions(clover2_fcu_bridge PRIVATE
  CLOVER2_FCU_BRIDGE_SERVER_BUILDING_LIBRARY
)
rclcpp_components_register_node(clover2_fcu_bridge
  PLUGIN "clover2_fcu_bridge::server"
  EXECUTABLE fcu_bridge
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
