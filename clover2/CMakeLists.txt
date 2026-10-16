cmake_minimum_required(VERSION 3.8)
project(clover2)

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

install(
  DIRECTORY examples
  DESTINATION share/${PROJECT_NAME}
)

ament_auto_package()
