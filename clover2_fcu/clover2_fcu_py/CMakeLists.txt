cmake_minimum_required(VERSION 3.8)
project(clover2_fcu_py)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)

ament_auto_find_build_dependencies()

ament_python_install_package(clover2_fcu)

set(${PROJECT_NAME}_sources
    src/_bind.cpp
    src/client.cpp
    src/settings.cpp
    src/capability/all.cpp
    src/capability/commands.cpp
    src/capability/system_state.cpp
)

pybind11_add_module(clover2_fcu_py ${${PROJECT_NAME}_sources})

target_include_directories(${PROJECT_NAME} PRIVATE ${clover2_fcu_INCLUDE_DIRS})
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${clover2_fcu_LIBRARIES}
        Eigen3::Eigen
        yaml-cpp
        rclcpp::rclcpp
)
target_include_directories(${PROJECT_NAME} PRIVATE include)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION "${PYTHON_INSTALL_DIR}/clover2_fcu"
)

install(
  DIRECTORY include/clover2_fcu_py
  DESTINATION include
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
