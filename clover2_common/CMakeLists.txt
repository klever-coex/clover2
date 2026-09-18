cmake_minimum_required(VERSION 3.8)
project(clover2_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(ament_cmake_python REQUIRED)

ament_auto_find_build_dependencies()

set(CLOVER2_VERSION_COMPOSED "$ENV{CLOVER2_VERSION}")
if("${CLOVER2_VERSION_COMPOSED}" STREQUAL "")
  set(CLOVER2_VERSION_COMPOSED "${${PROJECT_NAME}_VERSION}+dirty")
endif()
message(STATUS "clover2_common: baking version '${CLOVER2_VERSION_COMPOSED}'")

set(CLOVER2_GENERATED_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
  "include/clover2_common/version.hpp.in"
  "${CLOVER2_GENERATED_INCLUDE}/clover2_common/version.hpp"
  @ONLY
)

set(CLOVER2_GENERATED_PYTHON "${CMAKE_CURRENT_BINARY_DIR}/generated/python")
configure_file(
  python/_version.py.in
  "${CLOVER2_GENERATED_PYTHON}/clover2_common/_version.py"
  @ONLY
)

file(GLOB_RECURSE library_files "src/*.cpp")

ament_auto_add_library(${PROJECT_NAME} SHARED ${library_files})
target_include_directories(${PROJECT_NAME} PRIVATE "${CLOVER2_GENERATED_INCLUDE}")

ament_python_install_package(${PROJECT_NAME}
  PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/clover2_common"
)

ament_python_install_module(
  "${CLOVER2_GENERATED_PYTHON}/clover2_common/_version.py"
  DESTINATION_SUFFIX clover2_common
)

install(FILES "${CLOVER2_GENERATED_INCLUDE}/clover2_common/version.hpp"
  DESTINATION include/${PROJECT_NAME}
)

ament_export_include_directories(include)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
