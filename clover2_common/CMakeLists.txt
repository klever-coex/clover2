cmake_minimum_required(VERSION 3.8)
project(clover2_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(diagnostic_updater REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

set(dependencies
  diagnostic_updater
  lifecycle_msgs
  rclcpp
  rclcpp_lifecycle
)

set(CLOVER2_VERSION_COMPOSED "$ENV{CLOVER2_VERSION}")
if("${CLOVER2_VERSION_COMPOSED}" STREQUAL "")
  ament_package_xml()
  set(CLOVER2_VERSION_COMPOSED "${${PROJECT_NAME}_VERSION}")
endif()
message(STATUS "clover2_common: baking version '${CLOVER2_VERSION_COMPOSED}'")

set(CLOVER2_GENERATED_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
  include/clover2_common/version.hpp.in
  "${CLOVER2_GENERATED_INCLUDE}/clover2_common/version.hpp"
  @ONLY
)

set(CLOVER2_GENERATED_PYTHON "${CMAKE_CURRENT_BINARY_DIR}/generated/python")
configure_file(
  python/_version.py.in
  "${CLOVER2_GENERATED_PYTHON}/clover2_common/_version.py"
  @ONLY
)

include_directories(include "${CLOVER2_GENERATED_INCLUDE}")

set(library_name ${PROJECT_NAME})

file(GLOB_RECURSE library_files "src/*.cpp")
add_library(${library_name} SHARED
  ${library_files}
)

ament_target_dependencies(${library_name} ${dependencies})

install(TARGETS
  ${library_name}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
  PATTERN "*.in" EXCLUDE
)

install(FILES "${CLOVER2_GENERATED_INCLUDE}/clover2_common/version.hpp"
  DESTINATION include/${PROJECT_NAME}
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)

ament_python_install_package(${PROJECT_NAME}
  PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/clover2_common"
)

ament_python_install_module(
  "${CLOVER2_GENERATED_PYTHON}/clover2_common/_version.py"
  DESTINATION_SUFFIX clover2_common
)

ament_export_include_directories(include)
ament_export_libraries(${library_name})
ament_export_dependencies(${dependencies})
ament_export_include_directories("include/${PROJECT_NAME}")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "cmake/clover2_common_macros.cmake"
)
