cmake_minimum_required(VERSION 3.8)
project(clover2_backend)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)

find_package(Drogon REQUIRED)
find_package(Threads REQUIRED)

include_directories(include)

set(dependencies
  rclcpp
  rclcpp_components
)

file(GLOB_RECURSE backend_library_files "src/backend/*.cpp")

add_library(clover2_backend_server ${backend_library_files})
add_library(clover2::backend::server ALIAS clover2_backend_server)

target_compile_features(clover2_backend_server PUBLIC c_std_99 cxx_std_17)

target_include_directories(clover2_backend_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_link_libraries(clover2_backend_server
  Drogon::Drogon
  Threads::Threads
)

target_link_options(clover2_backend_server PRIVATE
  -Wl,--no-as-needed
)

target_compile_definitions(clover2_backend_server PRIVATE "CLOVER2_BACKEND_BUILDING_LIBRARY")
ament_target_dependencies(clover2_backend_server
  ${dependencies}
)

rclcpp_components_register_nodes(clover2_backend_server clover2::backend::server)

# install(TARGETS clover2_backend_server
#   DESTINATION lib/${PROJECT_NAME}
# )

install(
  TARGETS clover2_backend_server
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib      # Static libraries
  LIBRARY DESTINATION lib      # Shared libraries
  RUNTIME DESTINATION bin      # Executables (Windows)
)

# Export package dependencies
ament_export_dependencies(${dependencies})

# Export include directories
ament_export_include_directories(
  "include/${PROJECT_NAME}"
)

ament_export_libraries(
  clover2_backend_server
)

# Export CMake targets
ament_export_targets(
  export_${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
